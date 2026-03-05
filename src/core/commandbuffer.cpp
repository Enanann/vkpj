#include "commandbuffer.hpp"

#include "device.hpp"
#include "commandpool.hpp"
#include "pipeline/graphics_pipeline.hpp"
#include "swapchain.hpp"
#include "vulkan/vulkan.hpp"

CommandBuffer::CommandBuffer(const VulkanDevice& device, Swapchain& swapchain, const CommandPool& commandPool, const GraphicsPipeline& gPipeline) 
    : mVulkanDevice{device}
    , mSwapchain{swapchain}
    , mCommandPool{commandPool} 
    , mGraphicsPipeline{gPipeline}
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

    mCommandBuffer.draw(3, 1, 0, 0);

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

vk::raii::CommandBuffer& CommandBuffer::getVkHandle() {
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
