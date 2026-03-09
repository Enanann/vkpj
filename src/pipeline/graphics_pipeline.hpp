#pragma once

#include <vulkan/vulkan_raii.hpp>

class VulkanDevice;
class Shader;
class Swapchain;
class DescriptorSetLayout;

class GraphicsPipeline {
public:
    GraphicsPipeline(const VulkanDevice&, Shader&, Swapchain&, DescriptorSetLayout&);
    const vk::raii::Pipeline& getVkHandle() const;
    const vk::raii::PipelineLayout& getLayout() const;

private:
    const VulkanDevice& mVulkanDevice;
    Shader& mShader;
    Swapchain& mSwapchain;
    DescriptorSetLayout& mDescriptorSetLayout;

    vk::raii::PipelineLayout mPipelineLayout{nullptr};
    vk::raii::Pipeline mGraphicsPipeline{nullptr};
};
