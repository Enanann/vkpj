#include "renderer.hpp"
#include "buffer.hpp"
#include "commandbuffer.hpp"
#include "descriptor_set.hpp"
#include "device.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "image_loader.hpp"
#include "vulkan/vulkan.hpp"
#include "constant.hpp"

#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstring>
#include <cassert>
#include <stdexcept>
#include <cstdint>
// #include <iostream>
// #include <chrono>


Renderer::Renderer(Window& window) 
    : mWindow{window}
    , mInstance{}
    , mGLFWSurface{mInstance, mWindow}
    , mVulkanDevice{mInstance, mGLFWSurface}
    , mSwapchain{mVulkanDevice, mGLFWSurface, mWindow}
    , mShader{mVulkanDevice, "build/src/shaders/slang.spv"}
    , mDescriptorSetLayout(mVulkanDevice, {.bindings = {{0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex}, 
                                                        {1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment}}})
    , mGraphicsPipeline{mVulkanDevice, mShader, mSwapchain, mDescriptorSetLayout}
    , mCommandPool{mVulkanDevice}
    , mDescriptorPool(mVulkanDevice, {.sizes = {{vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT}, 
                                                {vk::DescriptorType::eCombinedImageSampler, MAX_FRAMES_IN_FLIGHT}}, .maxSets = MAX_FRAMES_IN_FLIGHT})
    , mImage(mVulkanDevice, mCommandPool, {ImageLoader::loadImageFromPath("textures/Ichika6.jpeg")})
    , mImGuiSystem(this)
    // , mCommandBuffer{mVulkanDevice, mSwapchain, mCommandPool, mGraphicsPipeline}
    {
    BufferConfig vertexConfig{
        .usage         = vk::BufferUsageFlagBits::eVertexBuffer,
        .memProperties = vk::MemoryPropertyFlagBits::eDeviceLocal
    };
    mVertexBuffer.emplace(Buffer::createBuffer(mVulkanDevice, mCommandPool, vertexConfig, gVertices));

    BufferConfig indexConfig{
        .usage         = vk::BufferUsageFlagBits::eIndexBuffer,
        .memProperties = vk::MemoryPropertyFlagBits::eDeviceLocal
    };
    mIndexBuffer.emplace(Buffer::createBuffer(mVulkanDevice, mCommandPool, indexConfig, gIndices));

    vk::DeviceSize uniformBufferSize{sizeof(UniformBufferObject)};
    vk::SemaphoreTypeCreateInfo semaphoreTypeCreateInfo{
        .semaphoreType = vk::SemaphoreType::eTimeline,
        .initialValue  = 0
    };
    mFrameDatas.clear();
    mFrameDatas.reserve(MAX_FRAMES_IN_FLIGHT);
    mDescriptorSets.clear();
    mDescriptorSets.reserve(MAX_FRAMES_IN_FLIGHT);
    for (size_t i{0}; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        mUniformBuffers.emplace_back(Buffer::createUniformBuffer(mVulkanDevice, uniformBufferSize));
        mDescriptorSets.emplace_back(mVulkanDevice, mDescriptorSetLayout, mDescriptorPool);
        mFrameDatas.emplace_back(
            CommandBuffer(mVulkanDevice, mSwapchain, mCommandPool, mGraphicsPipeline, mVertexBuffer, mIndexBuffer, mDescriptorSets[i]),
            vk::raii::Semaphore(mVulkanDevice.getVkHandle(), {.pNext = &semaphoreTypeCreateInfo}),
            0,
            vk::raii::Semaphore(mVulkanDevice.getVkHandle(), vk::SemaphoreCreateInfo())
        );
        const DescriptorBufferUpdateConfig bufferUpdateConfig{
            .binding = 0,
            .type = vk::DescriptorType::eUniformBuffer,
            .buffer = mUniformBuffers[i]->getVkHandle(),
            .size = sizeof(UniformBufferObject)
        };
        mDescriptorSets[i].updateBuffer(bufferUpdateConfig);

        const DescriptorImageUpdateConfig imageUpdateConfig{
            .binding = 1,
            .type = vk::DescriptorType::eCombinedImageSampler,
            .image = mImage,
            .layout = vk::ImageLayout::eShaderReadOnlyOptimal,
            .sampler = mImage.getSampler()
        };
        mDescriptorSets[i].updateImage(imageUpdateConfig);
    }

    for (auto i{0}; i < mSwapchain.getImages().size(); ++i) {
        mRenderFinishedSemaphores.emplace_back(mVulkanDevice.getVkHandle(), vk::SemaphoreCreateInfo());
    }
    _calculateScaling();
}

void Renderer::_calculateScaling() {
    float _w{static_cast<float>(mImage.getImageLoader().getResult().texWidth)};
    float _h{static_cast<float>(mImage.getImageLoader().getResult().texHeight)};
    float imageAspect{_w / _h};
    
    float scaleX = (imageAspect > 1) ? 1 : imageAspect;
    float scaleY = (imageAspect > 1) ? 1 / imageAspect : 1;

    mScale = glm::vec2(scaleX, scaleY);
    // std::cout << mScale.x << ' ' << mScale.y << '\n';
}

void Renderer::drawImGui() {
    mImGuiSystem.newFrame();
    mImGuiSystem.render();
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
    
    UniformBufferObject ubo{};
    // static auto startTime{std::chrono::high_resolution_clock::now()};

    // auto currentTime{std::chrono::high_resolution_clock::now()};
    // float time{std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count()};
    
    ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(mPan, 0.0f));
    ubo.model = glm::scale(ubo.model, glm::vec3(mScale, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.proj = glm::perspective(glm::radians(mZoom), static_cast<float>(mSwapchain.getExtent().width) / static_cast<float>(mSwapchain.getExtent().height), 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;
    mUniformBuffers[mCurrentFrame]->update(ubo);
    
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
        return;
    } else {
        // There are no other success code than eSuccess; on any error code, presentKHR already threw an exception
        assert(result == vk::Result::eSuccess);
    }

    mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::setPan(glm::vec2& v) {
    mPan = glm::vec2(v);
}

void Renderer::setZoom(float z) {
    mZoom = z;
}

const Window& Renderer::getWindow() const {
    return mWindow;
}

const Instance& Renderer::getInstance() const {
    return mInstance;
}

const VulkanDevice& Renderer::getDevice() const {
    return mVulkanDevice;
}

const Swapchain& Renderer::getSwapchain() const {
    return mSwapchain;
}
