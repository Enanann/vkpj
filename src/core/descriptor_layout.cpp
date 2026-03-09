#include "descriptor_layout.hpp"
#include "device.hpp"

DescriptorSetLayout::DescriptorSetLayout(const VulkanDevice& device, const DescriptorSetLayoutConfig& config) : mVulkanDevice{device} {
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    for (const auto binding : config.bindings) {
        bindings.push_back({
            .binding            = binding.binding,
            .descriptorType     = binding.type,
            .descriptorCount    = 1,
            .stageFlags         = binding.stages,
            .pImmutableSamplers = nullptr
        });
    }

    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings    = bindings.data()
    };

    mDescriptorSetLayout = vk::raii::DescriptorSetLayout(mVulkanDevice.getVkHandle(), descriptorSetLayoutCreateInfo);
}

const vk::raii::DescriptorSetLayout& DescriptorSetLayout::getVkHandle() const {
    return mDescriptorSetLayout;
}
