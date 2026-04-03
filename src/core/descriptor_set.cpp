#include "descriptor_set.hpp"

#include "descriptor_layout.hpp"
#include "descriptor_pool.hpp"
#include "device.hpp"
#include "sampler.hpp"
#include "image.hpp"
#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>

DescriptorSet::DescriptorSet(const VulkanDevice& device, const DescriptorSetLayout& layout, const DescriptorPool& pool)
    : mVulkanDevice{device}
    , mLayout{layout}
    , mPool{pool}
{
    vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool     = *mPool.getVkHandle(),
        .descriptorSetCount = 1,
        .pSetLayouts        = &*mLayout.getVkHandle()
    };

    mDescriptorSet = std::move(vk::raii::DescriptorSets(mVulkanDevice.getVkHandle(), allocInfo).front());
}

void DescriptorSet::updateBuffer(const DescriptorBufferUpdateConfig& config) {
    vk::DescriptorBufferInfo bufferInfo{
        .buffer = config.buffer,
        .offset = 0,
        .range  = config.size
    };

    vk::WriteDescriptorSet descriptorWrite{
        .dstSet          = mDescriptorSet,
        .dstBinding      = config.binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType  = config.type,
        .pBufferInfo     = &bufferInfo
    };
    mVulkanDevice.getVkHandle().updateDescriptorSets(descriptorWrite, {});
}

void DescriptorSet::updateImage(const DescriptorImageUpdateConfig& config) {
    vk::Sampler samplerHandle = nullptr;
    if (config.type == vk::DescriptorType::eCombinedImageSampler && config.sampler.has_value()) {
        samplerHandle = *config.sampler->get().getVkHandle();
    }

    vk::DescriptorImageInfo imageInfo{
        .sampler = samplerHandle,
        .imageView = config.image.getImageView(),
        .imageLayout = config.layout
    };

    vk::WriteDescriptorSet descriptorWrite{
        .dstSet          = mDescriptorSet,
        .dstBinding      = config.binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType  = config.type,
        .pImageInfo      = &imageInfo
    };
    mVulkanDevice.getVkHandle().updateDescriptorSets(descriptorWrite, {});
}

const vk::raii::DescriptorSet& DescriptorSet::getVkHandle() const {
    return mDescriptorSet;
}
