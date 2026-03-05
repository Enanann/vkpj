#include "renderer.hpp"
#include "commandbuffer.hpp"
#include "device.hpp"
#include "vulkan/vulkan.hpp"
#include "constant.hpp"

#include <cassert>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>
#include <stdexcept>
#include <cstdint>


Renderer::Renderer(Window& window) 
    : mWindow{window}
    , mInstance{}
    , mGLFWSurface{mInstance, mWindow}
    , mVulkanDevice{mInstance, mGLFWSurface}
    , mSwapchain{mVulkanDevice, mGLFWSurface, mWindow}
    , mShader{mVulkanDevice, "build/src/shaders/slang.spv"}
    , mGraphicsPipeline{mVulkanDevice, mShader, mSwapchain}
    , mCommandPool{mVulkanDevice}
    // , mCommandBuffer{mVulkanDevice, mSwapchain, mCommandPool, mGraphicsPipeline}
    {

    vk::SemaphoreTypeCreateInfo semaphoreTypeCreateInfo{
        .semaphoreType = vk::SemaphoreType::eTimeline,
        .initialValue  = 0
    };

    mFrameDatas.clear();
    mFrameDatas.reserve(MAX_FRAMES_IN_FLIGHT);
    for (auto i{0}; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        mFrameDatas.emplace_back(
            CommandBuffer(mVulkanDevice, mSwapchain, mCommandPool, mGraphicsPipeline),
            vk::raii::Semaphore(mVulkanDevice.getVkHandle(), {.pNext = &semaphoreTypeCreateInfo}),
            0,
            vk::raii::Semaphore(mVulkanDevice.getVkHandle(), vk::SemaphoreCreateInfo())
        );
    }

    for (auto i{0}; i < mSwapchain.getImages().size(); ++i) {
        mRenderFinishedSemaphores.emplace_back(mVulkanDevice.getVkHandle(), vk::SemaphoreCreateInfo());
    }
}

void Renderer::draw() {
    auto& frame{mFrameDatas[mCurrentFrame]};

    // Wait for the last frame to finish
    auto semaphoreResult = mVulkanDevice.getVkHandle().waitSemaphores(
        vk::SemaphoreWaitInfo{
            .semaphoreCount = 1,
            .pSemaphores    = &*frame.timelineSemaphore,
            .pValues        = &frame.lastSubmittedValue
        },
        UINT64_MAX
    );
    if (semaphoreResult != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to wait for semaphore");
    }
    ++frame.lastSubmittedValue;
    
    // Acquire next image, signal imageAvailableSemaphore
    auto [result, imageIndex] = mSwapchain.getVkHandle().acquireNextImage(UINT64_MAX, *frame.imageAvailableSemaphore, nullptr);
    if (result == vk::Result::eErrorOutOfDateKHR) {
        mSwapchain.recreate();
        return;
    } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
        throw std::runtime_error("Failed to acquire swapchain image");
    }

    frame.commandBuffer.getVkHandle().reset();
    frame.commandBuffer.record(imageIndex);

    vk::SemaphoreSubmitInfo waitImageAvailableSemaphore{
        .semaphore   = frame.imageAvailableSemaphore,
        .value       = 0,
        .stageMask   = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .deviceIndex = 0
    };

    vk::SemaphoreSubmitInfo signalTimeline{
        .semaphore   = *frame.timelineSemaphore,
        .value       = frame.lastSubmittedValue,
        .stageMask   = vk::PipelineStageFlagBits2::eAllGraphics,
        .deviceIndex = 0
    };
    vk::SemaphoreSubmitInfo signalRenderFinishedSemaphore{
        .semaphore   = mRenderFinishedSemaphores[imageIndex],
        .value       = 0,
        .stageMask   = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .deviceIndex = 0
    };
    std::array<vk::SemaphoreSubmitInfo, 2> semaphoreSubmitInfos{signalTimeline, signalRenderFinishedSemaphore};

    const vk::CommandBufferSubmitInfo commandBufferSubmitInfo{
        .commandBuffer = *frame.commandBuffer.getVkHandle(),
        .deviceMask    = 0
    };
    const vk::SubmitInfo2 submitInfo{
        .waitSemaphoreInfoCount   = 1,
        .pWaitSemaphoreInfos      = &waitImageAvailableSemaphore,
        .commandBufferInfoCount   = 1,
        .pCommandBufferInfos      = &commandBufferSubmitInfo,
        .signalSemaphoreInfoCount = static_cast<uint32_t>(semaphoreSubmitInfos.size()),
        .pSignalSemaphoreInfos    = semaphoreSubmitInfos.data()
    };

    mVulkanDevice.getGraphicsQueue().submit2(submitInfo);

    const vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &*mRenderFinishedSemaphores[imageIndex],
        .swapchainCount     = 1,
        .pSwapchains        = &*mSwapchain.getVkHandle(),
        .pImageIndices      = &imageIndex
    };
    result = mVulkanDevice.getPresentQueue().presentKHR(presentInfo);
    if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR || mWindow.getResizeStatus()) {
        mWindow.setResizeStatus(false);
        mSwapchain.recreate();
    } else {
        // There are no other success code than eSuccess; on any error code, presentKHR already threw an exception
        assert(result == vk::Result::eSuccess);
    }

    mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

const VulkanDevice& Renderer::getDevice() const {
    return mVulkanDevice;
}
