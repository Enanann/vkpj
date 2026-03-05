#include "commandpool.hpp"

#include "device.hpp"

CommandPool::CommandPool(const VulkanDevice& device) : mVulkanDevice{device} {
    vk::CommandPoolCreateInfo commandPoolCreateInfo{
        .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = mVulkanDevice.getGraphicsFamilyIndex()
    };
    mCommandPool = vk::raii::CommandPool(mVulkanDevice.getVkHandle(), commandPoolCreateInfo);
}

const vk::raii::CommandPool& CommandPool::getVkHandle() const {
    return mCommandPool;
}
