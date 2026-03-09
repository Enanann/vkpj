#pragma once

#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>

class VulkanDevice;

struct DescriptorPoolSize {
    vk::DescriptorType type;
    uint32_t count;
};

struct DescriptorPoolConfig {
    const std::vector<DescriptorPoolSize>& sizes;
    uint32_t maxSets;
};

class DescriptorPool {
public:
    DescriptorPool(const VulkanDevice&, DescriptorPoolConfig);

    const vk::raii::DescriptorPool& getVkHandle() const;
private:
    const VulkanDevice& mVulkanDevice;

    vk::raii::DescriptorPool mDescriptorPool{nullptr};
};
