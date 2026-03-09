#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

class VulkanDevice;
class DescriptorSetLayout;
class DescriptorPool;

struct DescriptorBufferUpdateConfig {
    uint32_t binding;
    vk::DescriptorType type;
    const vk::raii::Buffer& buffer;
    vk::DeviceSize size;
};

class DescriptorSet {
public:
    DescriptorSet(const VulkanDevice&, const DescriptorSetLayout&, const DescriptorPool&);

    const vk::raii::DescriptorSet& getVkHandle() const;

    void updateBuffer(const DescriptorBufferUpdateConfig&);
private:
    const VulkanDevice&        mVulkanDevice;
    const DescriptorSetLayout& mLayout;
    const DescriptorPool&      mPool;

    vk::raii::DescriptorSet mDescriptorSet{nullptr};
};
