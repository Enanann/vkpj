#pragma once

#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>

#include <cstdint>
class VulkanDevice;

struct DescriptorSetLayoutBindingConfig {
    uint32_t binding;
    vk::DescriptorType type;
    vk::ShaderStageFlagBits stages;
};

struct DescriptorSetLayoutConfig {
    const std::vector<DescriptorSetLayoutBindingConfig>& bindings;
};

class DescriptorSetLayout {
public:
    DescriptorSetLayout(const VulkanDevice&, const DescriptorSetLayoutConfig&);

    const vk::raii::DescriptorSetLayout& getVkHandle() const;
private:
    const VulkanDevice& mVulkanDevice;

    vk::raii::DescriptorSetLayout mDescriptorSetLayout{nullptr};
};
