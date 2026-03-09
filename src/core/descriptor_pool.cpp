#include "descriptor_pool.hpp"

#include "device.hpp"

DescriptorPool::DescriptorPool(const VulkanDevice& device, DescriptorPoolConfig config) : mVulkanDevice{device} {
    std::vector<vk::DescriptorPoolSize> poolSizes;
    poolSizes.reserve(config.sizes.size());
    for (auto p : config.sizes) {
        poolSizes.push_back({p.type, p.count});
    }
    
    vk::DescriptorPoolCreateInfo poolCreateInfo{
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = config.maxSets,
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data()
    };

    mDescriptorPool = vk::raii::DescriptorPool(mVulkanDevice.getVkHandle(), poolCreateInfo);
}

const vk::raii::DescriptorPool& DescriptorPool::getVkHandle() const {
    return mDescriptorPool;
}
