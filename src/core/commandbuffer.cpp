#include "commandbuffer.hpp"

#include "constant.hpp"
#include "device.hpp"
#include "commandpool.hpp"
#include "pipeline/graphics_pipeline.hpp"
#include "swapchain.hpp"
#include "buffer.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <stdexcept>
#include <vulkan/vulkan_raii.hpp>

CommandBuffer::CommandBuffer(const VulkanDevice& device, Swapchain& swapchain, const CommandPool& commandPool, const GraphicsPipeline& gPipeline, const std::optional<Buffer>& vertexBuffer, const std::optional<Buffer>& indexBuffer) 
    : mVulkanDevice{device}
    , mSwapchain{swapchain}
    , mCommandPool{commandPool} 
    , mGraphicsPipeline{gPipeline}
    , mVertexBuffer{vertexBuffer}
    , mIndexBuffer{indexBuffer}
{
    vk::CommandBufferAllocateInfo allocInfo{
        .commandPool        = mCommandPool.getVkHandle(),
        .level              = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };

    mCommandBuffer = std::move(vk::raii::CommandBuffers(mVulkanDevice.getVkHandle(), allocInfo).front());
}

void CommandBuffer::record(uint32_t imageIndex) {
    mCommandBuffer.begin({});

    transition_image_layout(
        imageIndex, 
        vk::ImageLayout::eUndefined, 
        vk::ImageLayout::eColorAttachmentOptimal, 
        {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,  
        vk::PipelineStageFlagBits2::eColorAttachmentOutput
    );

    vk::ClearValue clearColor{vk::ClearColorValue(0.39f, 0.58f, 0.93f, 1.0f)};
    vk::RenderingAttachmentInfo attachmentInfo{
        .imageView   = mSwapchain.getImageViews()[imageIndex],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp      = vk::AttachmentLoadOp::eClear,
        .storeOp     = vk::AttachmentStoreOp::eStore,
        .clearValue  = clearColor 
    };

    vk::RenderingInfo renderingInfo{
        .renderArea           = {.offset{0, 0}, .extent = mSwapchain.getExtent()},
        .layerCount           = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments    = &attachmentInfo
    };

    // start rendering
    mCommandBuffer.beginRendering(renderingInfo);

    mCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mGraphicsPipeline.getVkHandle());
    
    mCommandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(mSwapchain.getExtent().width), static_cast<float>(mSwapchain.getExtent().height)));
    mCommandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), mSwapchain.getExtent()));

    mCommandBuffer.bindVertexBuffers(0, *mVertexBuffer->getVkHandle(), {0});
    mCommandBuffer.bindIndexBuffer(*mIndexBuffer->getVkHandle(), 0, vk::IndexType::eUint16);

    mCommandBuffer.drawIndexed(gIndices.size(), 1, 0, 0, 0);

    mCommandBuffer.endRendering();

    // transition image layout to optimal for presenting
    transition_image_layout(
        imageIndex,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,                 // srcAccessMask
        {},  														// dstAccessMask 
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,         // srcStage
        vk::PipelineStageFlagBits2::eBottomOfPipe                   // dstStage
    );

    mCommandBuffer.end();
}

const vk::raii::CommandBuffer& CommandBuffer::getVkHandle() const {
    return mCommandBuffer;
}


void CommandBuffer::transition_image_layout(
    uint32_t imageIndex,
    vk::ImageLayout oldLayout,
    vk::ImageLayout newLayout,
    vk::AccessFlags2 srcAccessMask,
    vk::AccessFlags2 dstAccessMask,
    vk::PipelineStageFlags2 srcStageMask,
    vk::PipelineStageFlags2 dstStageMask
) {
    vk::ImageMemoryBarrier2 barrier = {
        .srcStageMask  = srcStageMask,
        .srcAccessMask = srcAccessMask,
        .dstStageMask  = dstStageMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout     = oldLayout,
        .newLayout     = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = mSwapchain.getImages()[imageIndex],
        .subresourceRange = {
            .aspectMask     = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1
        }
    };
    vk::DependencyInfo dependencyInfo = {
        .dependencyFlags         = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers    = &barrier
    };
    mCommandBuffer.pipelineBarrier2(dependencyInfo);
}

SingleTimeCommandBuffer::SingleTimeCommandBuffer(const VulkanDevice& device, const CommandPool& commandPool) 
    : mVulkanDevice{device}
    , mCommandPool{commandPool} 
{
    vk::CommandBufferAllocateInfo allocInfo{
        .commandPool        = mCommandPool.getVkHandle(),
        .level              = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };

    mCommandBuffer = std::move(vk::raii::CommandBuffers(mVulkanDevice.getVkHandle(), allocInfo).front());

    mCommandBuffer.begin(vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
}

void SingleTimeCommandBuffer::executeAndWait() {
    mCommandBuffer.end();
    
    vk::raii::Fence setupFence(mVulkanDevice.getVkHandle(), vk::FenceCreateInfo{.flags = vk::FenceCreateFlags{}});

    vk::SubmitInfo submitInfo{};
    submitInfo.setCommandBuffers(*mCommandBuffer);
    mVulkanDevice.getGraphicsQueue().submit(submitInfo, *setupFence);
    
    auto fenceResult = mVulkanDevice.getVkHandle().waitForFences(*setupFence, vk::True, UINT64_MAX);
    if (fenceResult != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to wait for fence");
    }
    
    mCommandBuffer.reset();
}

const vk::raii::CommandBuffer& SingleTimeCommandBuffer::getVkHandle() const {
    return mCommandBuffer;
}

// void SingleTimeCommandBuffer::executeAndWait() {
//     mCommandBuffer.end();

//     vk::SemaphoreTypeCreateInfo semaphoreCreateInfo{
//         .semaphoreType = vk::SemaphoreType::eTimeline,
//         .initialValue  = 0
//     };
//     vk::raii::Semaphore timelineSemaphore(mVulkanDevice.getVkHandle(), {.pNext = &semaphoreCreateInfo});
//     uint64_t waitSemaphore{1};
//     vk::SemaphoreSubmitInfo signalTimeline{
//         .semaphore   = timelineSemaphore,
//         .value       = waitSemaphore,
//         .stageMask   = vk::PipelineStageFlagBits2::eAllCommands,
//         .deviceIndex = 0
//     };

//     const vk::CommandBufferSubmitInfo commandBufferSubmitInfo{
//         .commandBuffer = *mCommandBuffer,
//         .deviceMask    = 0
//     };
//     const vk::SubmitInfo2 submitInfo{
//         .waitSemaphoreInfoCount   = 0,
//         .pWaitSemaphoreInfos      = nullptr,
//         .commandBufferInfoCount   = 1,
//         .pCommandBufferInfos      = &commandBufferSubmitInfo,
//         .signalSemaphoreInfoCount = 1,
//         .pSignalSemaphoreInfos    = &signalTimeline
//     };
//     mVulkanDevice.getGraphicsQueue().submit2(submitInfo);

//     auto result = mVulkanDevice.getVkHandle().waitSemaphores({.semaphoreCount = 1, .pSemaphores = &*timelineSemaphore, .pValues = &waitSemaphore}, UINT64_MAX);
//     if (result != vk::Result::eSuccess) {
//         throw std::runtime_error("Failed to wait for timeline semaphore");
//     }
// }
