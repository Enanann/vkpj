#pragma once

#include "pipeline/graphics_pipeline.hpp"
#include <vulkan/vulkan_raii.hpp>

class VulkanDevice;
class Swapchain;
class CommandPool;

class CommandBuffer {
public:
    CommandBuffer(const VulkanDevice&, Swapchain&, const CommandPool&, const GraphicsPipeline&);

    void record(uint32_t imageIndex);

    vk::raii::CommandBuffer& getVkHandle();

private:
    void transition_image_layout(
        uint32_t imageIndex,
        vk::ImageLayout oldLayout,
        vk::ImageLayout newLayout,
        vk::AccessFlags2 srcAccessMask,
        vk::AccessFlags2 dstAccessMask,
        vk::PipelineStageFlags2 srcStageMask,
        vk::PipelineStageFlags2 dstStageMask
    );

    const VulkanDevice&     mVulkanDevice;
    Swapchain&              mSwapchain;
    const CommandPool&      mCommandPool;
    const GraphicsPipeline& mGraphicsPipeline;

    vk::raii::CommandBuffer mCommandBuffer{nullptr};
};
