#pragma once

#include <vulkan/vulkan_raii.hpp>
class VulkanDevice;
class Shader;
class DescriptorSetLayout;

class ComputePipeline {
public:
    ComputePipeline(const VulkanDevice&, Shader&, DescriptorSetLayout&);

    const vk::raii::Pipeline& getVkHandle() const;
    const vk::raii::PipelineLayout& getLayout() const;

private:
    const VulkanDevice&  mVulkanDevice;
    Shader&              mShader;
    DescriptorSetLayout& mDescriptorSetLayout;

    vk::raii::PipelineLayout mPipelineLayout{nullptr};
    vk::raii::Pipeline       mPipeline{nullptr};
};
