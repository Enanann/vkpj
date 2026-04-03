#include "compute_pipeline.hpp"
#include "descriptor_layout.hpp"
#include "device.hpp"
#include "shader.hpp"
#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>

ComputePipeline::ComputePipeline(const VulkanDevice& device, Shader& shader, DescriptorSetLayout& descriptorSetLayout)
    : mVulkanDevice{device}
    , mShader{shader}
    , mDescriptorSetLayout{descriptorSetLayout}
{
    vk::PipelineShaderStageCreateInfo computeStage{
        .stage = vk::ShaderStageFlagBits::eCompute,
        .module = mShader.getShaderModule(),
        .pName = "compMain"
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{
        .setLayoutCount = 1,
        .pSetLayouts = &*descriptorSetLayout.getVkHandle(),
        .pushConstantRangeCount = 0
    };

    mPipelineLayout = vk::raii::PipelineLayout(mVulkanDevice.getVkHandle(), pipelineLayoutCreateInfo);

    vk::ComputePipelineCreateInfo computePipelineCreateInfo{
        .stage = computeStage,
        .layout = mPipelineLayout
    };

    mPipeline = vk::raii::Pipeline(mVulkanDevice.getVkHandle(), nullptr, computePipelineCreateInfo);
}

const vk::raii::PipelineLayout& ComputePipeline::getLayout() const {
    return mPipelineLayout;
}

const vk::raii::Pipeline& ComputePipeline::getVkHandle() const {
    return mPipeline;
}
