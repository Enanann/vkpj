#include "buffer.hpp"
#include "image.hpp"
#include "commandbuffer.hpp"
#include "commandpool.hpp"
#include "device.hpp"
#include "vulkan/vulkan.hpp"

#include <cstdint>
#include <stdexcept>
#include <vulkan/vulkan_raii.hpp>

Buffer::Buffer(const VulkanDevice& device, vk::DeviceSize size, const BufferConfig& config) : mVulkanDevice{device} {
    vk::BufferCreateInfo bufferCreateInfo{
        .size = size,
        .usage = config.usage,
        .sharingMode = vk::SharingMode::eExclusive
    };

    mBuffer = vk::raii::Buffer(mVulkanDevice.getVkHandle(), bufferCreateInfo);

    vk::MemoryRequirements memReq{mBuffer.getMemoryRequirements()};
    vk::MemoryAllocateInfo memoryAllocInfo{
        .allocationSize  = memReq.size,
        .memoryTypeIndex = findMemoryType(mVulkanDevice.getPhysicalDeviceHandle(), memReq.memoryTypeBits, config.memProperties) 
    };
    mMemory = vk::raii::DeviceMemory(mVulkanDevice.getVkHandle(), memoryAllocInfo);
    mBuffer.bindMemory(*mMemory, 0);
    if (config.usage == vk::BufferUsageFlagBits::eUniformBuffer) {
        mMemoryMapped.emplace(mMemory.mapMemory(0, size)); // persistent mapping
    }
}

const vk::raii::Buffer&Buffer:: getVkHandle() const {
    return mBuffer;
}

const vk::raii::DeviceMemory& Buffer::getMemory() const {
    return mMemory;
}

uint32_t Buffer::findMemoryType(const vk::raii::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties memProperties{physicalDevice.getMemoryProperties()};
    for (uint32_t i{0}; i < memProperties.memoryTypeCount; ++i) {
        if ((typeFilter & (1 << i)) && ((memProperties.memoryTypes[i].propertyFlags & properties) == properties)) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type");
}

void Buffer::copy(const Buffer& buffer, const CommandPool& commandPool, vk::DeviceSize size) {
    SingleTimeCommandBuffer copyCommandBuffer(mVulkanDevice, commandPool);
    // src, dst, regions
    copyCommandBuffer.getVkHandle().copyBuffer(buffer.getVkHandle(), this->mBuffer, vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = size});
    copyCommandBuffer.executeAndWait();
}

void Buffer::copyToImage(SingleTimeCommandBuffer& copyCommandBuffer, Image* image, const CommandPool& commandPool, uint32_t width, uint32_t height) {
    vk::BufferImageCopy2 region{
        .bufferOffset      = 0,
        .bufferRowLength   = 0,
        .bufferImageHeight = 0,
        .imageSubresource  = {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
        .imageOffset       = {0, 0, 0},
        .imageExtent       = {width, height, 1}
    };
    vk::CopyBufferToImageInfo2 copyInfo{
        .srcBuffer      = mBuffer,
        .dstImage       = image->getImage(),
        .dstImageLayout = vk::ImageLayout::eTransferDstOptimal, 
        .regionCount    = 1,
        .pRegions       = &region
    };
    copyCommandBuffer.getVkHandle().copyBufferToImage2(copyInfo);
}
