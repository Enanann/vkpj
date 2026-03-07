#pragma once

#include "pipeline/graphics_pipeline.hpp"
#include <vulkan/vulkan_raii.hpp>

#include <optional>

class VulkanDevice;
class Swapchain;
class CommandPool;
class Buffer;

class CommandBuffer {
public:
    CommandBuffer(const VulkanDevice&, Swapchain&, const CommandPool&, const GraphicsPipeline&, const std::optional<Buffer>&, const std::optional<Buffer>&);

    void record(uint32_t imageIndex);

    const vk::raii::CommandBuffer& getVkHandle() const;

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
    const std::optional<Buffer>& mVertexBuffer;
    const std::optional<Buffer>& mIndexBuffer;

    vk::raii::CommandBuffer mCommandBuffer{nullptr};
};

class SingleTimeCommandBuffer {
public:
    SingleTimeCommandBuffer(const VulkanDevice&, const CommandPool&);
    void executeAndWait();
    const vk::raii::CommandBuffer& getVkHandle() const;
    // ~SingleTimeCommandBuffer();
private:
    const VulkanDevice& mVulkanDevice;
    const CommandPool&  mCommandPool;

    vk::raii::CommandBuffer mCommandBuffer{nullptr};
};
