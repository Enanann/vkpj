#pragma once

#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

class VulkanDevice;
class DescriptorSetLayout;
class DescriptorPool;
class Sampler;
class Image;

struct DescriptorBufferUpdateConfig {
    uint32_t binding;
    vk::DescriptorType type;
    const vk::raii::Buffer& buffer;
    vk::DeviceSize size;
};

struct DescriptorImageUpdateConfig {
    uint32_t binding;
    vk::DescriptorType type;
    const Image& image;
    vk::ImageLayout layout;
    const Sampler& sampler;
};

class DescriptorSet {
public:
    DescriptorSet(const VulkanDevice&, const DescriptorSetLayout&, const DescriptorPool&);

    const vk::raii::DescriptorSet& getVkHandle() const;

    void updateBuffer(const DescriptorBufferUpdateConfig&);
    void updateImage(const DescriptorImageUpdateConfig&);
private:
    const VulkanDevice&        mVulkanDevice;
    const DescriptorSetLayout& mLayout;
    const DescriptorPool&      mPool;

    vk::raii::DescriptorSet mDescriptorSet{nullptr};
};
