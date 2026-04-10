#pragma once

#include <vulkan/vulkan_raii.hpp>
class VulkanDevice;
class Shader;
class DescriptorSetLayout;

struct ComputePipelineConfig {
    bool usePushConstant;
    uint32_t pushConstantSize;
};

class ComputePipeline {
public:
    ComputePipeline(const VulkanDevice&, Shader&, DescriptorSetLayout&, ComputePipelineConfig);

    const vk::raii::Pipeline& getVkHandle() const;
    const vk::raii::PipelineLayout& getLayout() const;

    bool mUsePushConstant;
private:
    const VulkanDevice&  mVulkanDevice;
    Shader&              mShader;
    DescriptorSetLayout& mDescriptorSetLayout;

    vk::raii::PipelineLayout mPipelineLayout{nullptr};
    vk::raii::Pipeline       mPipeline{nullptr};
};
