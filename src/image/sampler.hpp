#pragma once

#include <vulkan/vulkan_raii.hpp>

class VulkanDevice;

struct SamplerConfig {};

class Sampler {
public:
    Sampler(const VulkanDevice&, const SamplerConfig&);

    const vk::raii::Sampler& getVkHandle() const;
private:
    const VulkanDevice& mVulkanDevice;

    vk::raii::Sampler mSampler{nullptr};
};
