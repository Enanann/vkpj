#pragma once

#include <cstdint>
#include <vulkan/vulkan_raii.hpp>

#include <optional>

class VulkanDevice;
class Swapchain;
class CommandPool;
class GraphicsPipeline;
class ComputePipeline;
class Buffer;
class DescriptorSet;
class Image;

class CommandBuffer {
public:
    CommandBuffer(const VulkanDevice&, Swapchain&, const CommandPool&, const GraphicsPipeline&, const std::optional<Buffer>&, const std::optional<Buffer>&, const DescriptorSet&);

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

    const VulkanDevice&          mVulkanDevice;
    Swapchain&                   mSwapchain;
    const CommandPool&           mCommandPool;
    const GraphicsPipeline&      mGraphicsPipeline;
    const std::optional<Buffer>& mVertexBuffer;
    const std::optional<Buffer>& mIndexBuffer;
    const DescriptorSet&         mDescriptorSet;

    vk::raii::CommandBuffer mCommandBuffer{nullptr};
};

class ComputeCommandBuffer {
public:
    ComputeCommandBuffer(const VulkanDevice&, const CommandPool&);

    void setDispatchDimension(uint32_t w, uint32_t h/*, uint32_t z*/);
    void begin();
    void setPushConstant(const vk::raii::PipelineLayout& layout, uint32_t offset, uint32_t size, const void* data);
    void record(uint32_t imageIndex, Image&, const ComputePipeline&, DescriptorSet&);
    void end();
    void transition_image_layout(
        Image& img, 
        vk::PipelineStageFlags2 srcStageMask, 
        vk::AccessFlags2 srcAccessMask,
        vk::PipelineStageFlags2 dstStageMask, 
        vk::AccessFlags2 dstAccessMask,
        vk::ImageLayout oldLayout,
        vk::ImageLayout newLayout
    );

    const vk::raii::CommandBuffer& getVkHandle() const;
private:
    const VulkanDevice&    mVulkanDevice;
    const CommandPool&     mCommandPool;
    // const DescriptorSet&   mComputeDescriptorSet;

    uint32_t _width{};
    uint32_t _height{};

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
