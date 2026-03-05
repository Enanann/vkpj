#pragma once   

#include <vulkan/vulkan_raii.hpp>

class VulkanDevice;

class CommandPool {
public:
    CommandPool(const VulkanDevice&);

    const vk::raii::CommandPool& getVkHandle() const;

private:
    const VulkanDevice& mVulkanDevice;

    vk::raii::CommandPool mCommandPool{nullptr};
};
