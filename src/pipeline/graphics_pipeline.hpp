#pragma once

#include <vulkan/vulkan_raii.hpp>

class VulkanDevice;
class Shader;
class Swapchain;

class GraphicsPipeline {
public:
    GraphicsPipeline(const VulkanDevice&, Shader&, Swapchain&);
    const vk::raii::Pipeline& getVkHandle() const;

private:
    const VulkanDevice& mVulkanDevice;
    Shader& mShader;
    Swapchain& mSwapchain;

    vk::raii::PipelineLayout mPipelineLayout{nullptr};
    vk::raii::Pipeline mGraphicsPipeline{nullptr};
};
