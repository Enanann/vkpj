#include "sampler.hpp"

#include "device.hpp"

Sampler::Sampler(const VulkanDevice& device, const SamplerConfig& config) : mVulkanDevice{device} {
    auto properties{mVulkanDevice.getPhysicalDeviceHandle().getProperties()};
    vk::SamplerCreateInfo samplerCreateInfo{
        .magFilter        = vk::Filter::eLinear,
        .minFilter        = vk::Filter::eLinear,
        .mipmapMode       = vk::SamplerMipmapMode::eLinear,
        .addressModeU     = vk::SamplerAddressMode::eClampToBorder,
        .addressModeV     = vk::SamplerAddressMode::eClampToBorder,
        .addressModeW     = vk::SamplerAddressMode::eClampToBorder,
        .anisotropyEnable = vk::True,
        .maxAnisotropy    = properties.limits.maxSamplerAnisotropy
    };

    mSampler = vk::raii::Sampler(mVulkanDevice.getVkHandle(), samplerCreateInfo);
}

const vk::raii::Sampler& Sampler::getVkHandle() const {
    return mSampler;
}
