#include "image.hpp"

#include "buffer.hpp"
#include "commandbuffer.hpp"
#include "commandpool.hpp"
#include "device.hpp"
#include "image_loader.hpp"
#include "vulkan/vulkan.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>


Image::Image(const VulkanDevice& device, const CommandPool& commandPool, const ImageConfig& config) 
    : mVulkanDevice{device}
    , mCommandPool{commandPool}
    , mImageLoader{config.image} 
{
    const auto& image{config.image};
    const auto& imageRes{image.getResult()};

    BufferConfig stagingConfig{
        .usage         = vk::BufferUsageFlagBits::eTransferSrc,
        .memProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    };
    Buffer stagingBuffer{Buffer::createBuffer(mVulkanDevice, stagingConfig, image)};

    vk::ImageCreateInfo imageCreateInfo{
        .imageType     = vk::ImageType::e2D,
        .format        = config.format,
        .extent        = {static_cast<uint32_t>(imageRes.texWidth), static_cast<uint32_t>(imageRes.texHeight), 1},
        .mipLevels     = 1,
        .arrayLayers   = 1,
        .samples       = vk::SampleCountFlagBits::e1,
        .tiling        = config.tiling,
        .usage         = config.usage,
        .sharingMode   = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined
    };

    mImage = vk::raii::Image(mVulkanDevice.getVkHandle(), imageCreateInfo);
    vk::MemoryRequirements memReq{mImage.getMemoryRequirements()};
    vk::MemoryAllocateInfo memoryAllocateInfo{
        .allocationSize = memReq.size,
        .memoryTypeIndex = Buffer::findMemoryType(mVulkanDevice.getPhysicalDeviceHandle(), memReq.memoryTypeBits, config.properties)
    };
    mImageMemory = vk::raii::DeviceMemory(mVulkanDevice.getVkHandle(), memoryAllocateInfo);
    mImage.bindMemory(mImageMemory, 0);

    SingleTimeCommandBuffer singleTimeCommandBuffer(mVulkanDevice, mCommandPool);
    transitionImageLayout(singleTimeCommandBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    stagingBuffer.copyToImage(singleTimeCommandBuffer, this, mCommandPool, imageRes.texWidth, imageRes.texHeight);
    transitionImageLayout(singleTimeCommandBuffer, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
    singleTimeCommandBuffer.executeAndWait();

    vk::ImageViewCreateInfo imageViewCreateInfo{
        .image    = mImage,
        .viewType = vk::ImageViewType::e2D,
        .format   = config.format,
        .subresourceRange = {
            .aspectMask     = config.aspect,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1
        }
    };
    mImageView = vk::raii::ImageView(mVulkanDevice.getVkHandle(), imageViewCreateInfo);

    mSampler.emplace(Sampler(mVulkanDevice, {}));
}

Image::~Image() {
    if (mImageLoader.getResult().pixels) {
        stbi_image_free(mImageLoader.getResult().pixels);
    }
}

const ImageLoader& Image::getImageLoader() const {
    return mImageLoader;
}

const vk::raii::Image& Image::getImage() const {
    return mImage;
}

const vk::raii::ImageView& Image::getImageView() const {
    return mImageView;
}

const Sampler& Image::getSampler() const {
    return mSampler.value();
}

void Image::transitionImageLayout(SingleTimeCommandBuffer& commandBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
    vk::ImageMemoryBarrier2 barrier{
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .image     = mImage,
        // .srcQueueFamilyIndex = , // don't need to transfer ownership
        // .dstQueueFamilyIndex = , // don't need to transfer ownership
        .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
    };
    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits2::eTransferWrite;

        barrier.srcStageMask  = vk::PipelineStageFlagBits2::eTopOfPipe;
        barrier.dstStageMask  = vk::PipelineStageFlagBits2::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits2::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits2::eShaderRead;

        barrier.srcStageMask  = vk::PipelineStageFlagBits2::eTransfer;
        barrier.dstStageMask  = vk::PipelineStageFlagBits2::eFragmentShader; 
    } else {
        throw std::runtime_error("Unsupported layout transition");
    }

    vk::DependencyInfo dependencyInfo{
        .dependencyFlags = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier
    };

    commandBuffer.getVkHandle().pipelineBarrier2(dependencyInfo);
}

