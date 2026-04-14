#include "renderer.hpp"
#include "buffer.hpp"
#include "commandbuffer.hpp"
#include "commandpool.hpp"
#include "descriptor_set.hpp"
#include "device.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "image.hpp"
#include "image_loader.hpp"
#include "vulkan/vulkan.hpp"
#include "constant.hpp"

#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>

#include <array>
#include <cstddef>
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
    , mShader{mVulkanDevice, "build/src/shaders/shader_base.spv"}
    , mDescriptorSetLayout(mVulkanDevice, {.bindings = {{0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex}, 
                                                        {1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment}}})
    , mComputeDescriptorSetLayout(mVulkanDevice, {.bindings = {{0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute}, 
                                                               {1, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute}}})    
    // , mEffectRegistry{}
    , mGraphicsPipeline{mVulkanDevice, mShader, mSwapchain, mDescriptorSetLayout}
    , mCommandPool{mVulkanDevice}
    , mDescriptorPool(mVulkanDevice, {.sizes = {{vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT}, 
                                                {vk::DescriptorType::eCombinedImageSampler, MAX_FRAMES_IN_FLIGHT * 4},
                                                {vk::DescriptorType::eStorageImage, MAX_FRAMES_IN_FLIGHT * 3}}, .maxSets = MAX_FRAMES_IN_FLIGHT * 4})
    // , mImage(mVulkanDevice, mCommandPool, {ImageLoader::loadImageFromPath("textures/Ichika6.jpeg")})
    , mImGuiSystem(this)
    // , mCommandBuffer{mVulkanDevice, mSwapchain, mCommandPool, mGraphicsPipeline}
    {
    // mEffects.emplace_back(Effect(mVulkanDevice, "build/src/shaders/vignette.spv", mComputeDescriptorSetLayout));
    // mEffects.emplace_back(Effect(mVulkanDevice, "build/src/shaders/vignette.spv", mComputeDescriptorSetLayout));
    // mEffects.emplace_back(Effect(mVulkanDevice, "build/src/shaders/vignette.spv", mComputeDescriptorSetLayout));
    // mEffects.emplace_back(Effect(mVulkanDevice, "build/src/shaders/vignette.spv", mComputeDescriptorSetLayout));
    // mEffects.emplace_back(Effect(mVulkanDevice, "build/src/shaders/grayscale.spv", mComputeDescriptorSetLayout));

    // mEffects.emplace_back(std::make_unique<Effect>(mVulkanDevice, mComputeDescriptorSetLayout, *mEffectRegistry.getByName("grayscale")));
    // mEffects.emplace_back(std::make_unique<Effect>(mVulkanDevice, mComputeDescriptorSetLayout, *mEffectRegistry.getByName("vignette")));
    addEffect("Grayscale");
    addEffect("Vignette");

    // mImage.emplace(mVulkanDevice, mCommandPool, ImageConfig{ImageLoader::loadImageFromPath("textures/Ichika6.jpeg")});
#ifdef __linux__
    mImage.emplace(mVulkanDevice, mCommandPool, ImageConfig{ImageLoader::loadImageFromPath("../../Downloads/wallpp/sky.jpeg")});
#endif
#ifdef _WIN64
    mImage.emplace(mVulkanDevice, mCommandPool, ImageConfig{ImageLoader::loadImageFromPath("W:/Download/want to marry.jpg")});
#endif

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
    mComputeDescriptorSetsInit.clear();
    mComputeDescriptorSetsInit.reserve(MAX_FRAMES_IN_FLIGHT);
    mComputeDescriptorSetsAtoB.clear();
    mComputeDescriptorSetsAtoB.reserve(MAX_FRAMES_IN_FLIGHT);
    mComputeDescriptorSetsBtoA.clear();
    mComputeDescriptorSetsBtoA.reserve(MAX_FRAMES_IN_FLIGHT);
    for (size_t i{0}; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        mUniformBuffers.emplace_back(Buffer::createUniformBuffer(mVulkanDevice, uniformBufferSize));
        mDescriptorSets.emplace_back(mVulkanDevice, mDescriptorSetLayout, mDescriptorPool);
        mComputeDescriptorSetsInit.emplace_back(mVulkanDevice, mComputeDescriptorSetLayout, mDescriptorPool);
        mComputeDescriptorSetsAtoB.emplace_back(mVulkanDevice, mComputeDescriptorSetLayout, mDescriptorPool);
        mComputeDescriptorSetsBtoA.emplace_back(mVulkanDevice, mComputeDescriptorSetLayout, mDescriptorPool);
        mFrameDatas.emplace_back(
            CommandBuffer(mVulkanDevice, mSwapchain, mCommandPool, mGraphicsPipeline, mVertexBuffer, mIndexBuffer, mDescriptorSets[i]),
            ComputeCommandBuffer(mVulkanDevice, mCommandPool),
            Image(mVulkanDevice, mCommandPool, ComputeImageConfig{mImage->getImageLoader().getResult().texWidth, mImage->getImageLoader().getResult().texHeight}),
            Image(mVulkanDevice, mCommandPool, ComputeImageConfig{mImage->getImageLoader().getResult().texWidth, mImage->getImageLoader().getResult().texHeight}),
            vk::raii::Semaphore(mVulkanDevice.getVkHandle(), {.pNext = &semaphoreTypeCreateInfo}),
            0,
            vk::raii::Semaphore(mVulkanDevice.getVkHandle(), vk::SemaphoreCreateInfo())
        );
        mFrameDatas[i].computeCommandBuffer.setDispatchDimension(mImage->getImageLoader().getResult().texWidth, mImage->getImageLoader().getResult().texHeight);

        mDescriptorSets[i].updateBuffer({
            .binding = 0,
            .type    = vk::DescriptorType::eUniformBuffer,
            .buffer  = mUniformBuffers[i]->getVkHandle(),
            .size    = sizeof(UniformBufferObject)
        });

        // Descriptor set for first pass
        mComputeDescriptorSetsInit[i].updateImage({
            .binding = 0,
            .type    = vk::DescriptorType::eCombinedImageSampler,
            .image   = *mImage,
            .layout  = vk::ImageLayout::eShaderReadOnlyOptimal,
            .sampler = mImage->getSampler()
        });
        mComputeDescriptorSetsInit[i].updateImage({
            .binding = 1,
            .type    = vk::DescriptorType::eStorageImage,
            .image   = *mFrameDatas[i].ping,
            .layout  = vk::ImageLayout::eGeneral,
            .sampler = mFrameDatas[i].ping->getSampler()
        });

        // Descriptor set for ping -> pong
        mComputeDescriptorSetsAtoB[i].updateImage({
            .binding = 0,
            .type    = vk::DescriptorType::eCombinedImageSampler,
            .image   = *mFrameDatas[i].ping,
            .layout  = vk::ImageLayout::eShaderReadOnlyOptimal,
            .sampler = mFrameDatas[i].ping->getSampler()
        });
        mComputeDescriptorSetsAtoB[i].updateImage({
            .binding = 1,
            .type    = vk::DescriptorType::eStorageImage,
            .image   = *mFrameDatas[i].pong,
            .layout  = vk::ImageLayout::eGeneral,
            .sampler = mFrameDatas[i].pong->getSampler()
        });

        // Descriptor set for pong -> ping
        mComputeDescriptorSetsBtoA[i].updateImage({
            .binding = 0,
            .type    = vk::DescriptorType::eCombinedImageSampler,
            .image   = *mFrameDatas[i].pong,
            .layout  = vk::ImageLayout::eShaderReadOnlyOptimal,
            .sampler = mFrameDatas[i].pong->getSampler()
        });
        mComputeDescriptorSetsBtoA[i].updateImage({
            .binding = 1,
            .type    = vk::DescriptorType::eStorageImage,
            .image   = *mFrameDatas[i].ping,
            .layout  = vk::ImageLayout::eGeneral,
            .sampler = mFrameDatas[i].ping->getSampler()
        });
    }

    for (auto i{0}; i < mSwapchain.getImages().size(); ++i) {
        mRenderFinishedSemaphores.emplace_back(mVulkanDevice.getVkHandle(), vk::SemaphoreCreateInfo());
    }
    _calculateScaling();
}

void Renderer::_calculateScaling() {
    float _w{static_cast<float>(mImage->getImageLoader().getResult().texWidth)};
    float _h{static_cast<float>(mImage->getImageLoader().getResult().texHeight)};
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

#define COMPUTE
#ifndef COMPUTE
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
#endif

#ifdef COMPUTE
// draw with compute here
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
    
    // Acquire next image from the swapchain, signal imageAvailableSemaphore
    auto [result, imageIndex] = mSwapchain.getVkHandle().acquireNextImage(UINT64_MAX, *frame.imageAvailableSemaphore, nullptr);
    if (result == vk::Result::eErrorOutOfDateKHR) {
        mSwapchain.recreate();
        return;
    } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
        throw std::runtime_error("Failed to acquire swapchain image");
    }
    
    uint64_t lastFrameValue = frame.lastSubmittedValue; 
    uint64_t computeFinishedValue = lastFrameValue;      // default value if no effect is enabled
    uint64_t graphicsFinishedValue = lastFrameValue + 1;

    UniformBufferObject ubo{};
    ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(mPan, 0.0f));
    ubo.model = glm::scale(ubo.model, glm::vec3(mScale, 1.0f));
    ubo.view  = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.proj  = glm::perspective(glm::radians(mZoom), static_cast<float>(mSwapchain.getExtent().width) / static_cast<float>(mSwapchain.getExtent().height), 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;
    mUniformBuffers[mCurrentFrame]->update(ubo);

    Image* currentImage = &mImage.value();
    Image* currentWrite{nullptr};
    DescriptorSet* currentSetToBind{nullptr};
    int totalActiveEffects{0};
    for (const auto& e : mEffects) {
        if (e->mIsEnabled) {
            totalActiveEffects++;
        }
    }
    if (totalActiveEffects > 0) {
        computeFinishedValue++;
        graphicsFinishedValue++;
        // Compute
        {

            frame.computeCommandBuffer.begin();
            int activeEffectIndex{0};        
            for (size_t i{0}; i < mEffects.size(); ++i) {
                if (!mEffects[i]->mIsEnabled) continue;
                
                if (activeEffectIndex == 0) {
                    currentSetToBind = &mComputeDescriptorSetsInit[mCurrentFrame];
                    currentWrite = &mFrameDatas[mCurrentFrame].ping.value();
                } else if (activeEffectIndex % 2 == 1) {
                    currentSetToBind = &mComputeDescriptorSetsAtoB[mCurrentFrame];
                    currentWrite = &mFrameDatas[mCurrentFrame].pong.value();
                } else {
                    currentSetToBind = &mComputeDescriptorSetsBtoA[mCurrentFrame];
                    currentWrite = &mFrameDatas[mCurrentFrame].ping.value();
                }
    
                if (mEffects[i]->usePushConstant()) {
                    frame.computeCommandBuffer.setPushConstant(mEffects[i]->getPipeline().getLayout(), 0, mEffects[i]->getFloatParamSize(), mEffects[i]->getParamsData().data());
                }
    
                frame.computeCommandBuffer.record(imageIndex, *currentWrite, mEffects[i]->getPipeline(), *currentSetToBind);
    
                currentImage = currentWrite;
                activeEffectIndex++;
            }
            frame.computeCommandBuffer.end();

            vk::SemaphoreSubmitInfo waitCompute{
                .semaphore   = *frame.timelineSemaphore,
                .value       = lastFrameValue,
                .stageMask   = vk::PipelineStageFlagBits2::eComputeShader,
                .deviceIndex = 0
            };

            vk::SemaphoreSubmitInfo signalCompute{
                .semaphore   = *frame.timelineSemaphore,
                .value       = computeFinishedValue,
                .stageMask   = vk::PipelineStageFlagBits2::eComputeShader,
                .deviceIndex = 0
            };

            const vk::CommandBufferSubmitInfo commandBufferSubmitInfo{
                .commandBuffer = *frame.computeCommandBuffer.getVkHandle(),
                .deviceMask    = 0
            };
            const vk::SubmitInfo2 computeSubmitInfo{
                .waitSemaphoreInfoCount   = 1,
                .pWaitSemaphoreInfos      = &waitCompute,
                .commandBufferInfoCount   = 1,
                .pCommandBufferInfos      = &commandBufferSubmitInfo,
                .signalSemaphoreInfoCount = 1,
                .pSignalSemaphoreInfos    = &signalCompute
            };
            mVulkanDevice.getGraphicsQueue().submit2(computeSubmitInfo);
        }
    }
    // Graphics
    {
        mDescriptorSets[mCurrentFrame].updateImage({
            .binding = 1,
            .type    = vk::DescriptorType::eCombinedImageSampler,
            .image   = *currentImage,
            .layout  = vk::ImageLayout::eShaderReadOnlyOptimal,
            .sampler = currentImage->getSampler()
        });

        frame.commandBuffer.getVkHandle().reset();
        frame.commandBuffer.record(imageIndex);
        
        vk::SemaphoreSubmitInfo waitImageAvailableSemaphore{
            .semaphore   = frame.imageAvailableSemaphore,
            .value       = 0,
            .stageMask   = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            .deviceIndex = 0
        };

        vk::SemaphoreSubmitInfo waitComputeFinishedSemaphore{
            .semaphore   = *frame.timelineSemaphore,
            .value       = computeFinishedValue,
            .stageMask   = vk::PipelineStageFlagBits2::eVertexShader | vk::PipelineStageFlagBits2::eFragmentShader,
            .deviceIndex = 0
        };
        std::array<vk::SemaphoreSubmitInfo, 2> waitSemaphoreSubmitInfos{waitImageAvailableSemaphore, waitComputeFinishedSemaphore};
        
        vk::SemaphoreSubmitInfo signalGraphicsFinished{
            .semaphore   = *frame.timelineSemaphore,
            .value       = graphicsFinishedValue,
            .stageMask   = vk::PipelineStageFlagBits2::eAllGraphics,
            .deviceIndex = 0
        };
        vk::SemaphoreSubmitInfo signalRenderFinishedSemaphore{
            .semaphore   = mRenderFinishedSemaphores[imageIndex],
            .value       = 0,
            .stageMask   = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            .deviceIndex = 0
        };
        std::array<vk::SemaphoreSubmitInfo, 2> signalSemaphoreSubmitInfos{signalGraphicsFinished, signalRenderFinishedSemaphore};
        
        const vk::CommandBufferSubmitInfo commandBufferSubmitInfo{
            .commandBuffer = *frame.commandBuffer.getVkHandle(),
            .deviceMask    = 0
        };
        const vk::SubmitInfo2 submitInfo{
            .waitSemaphoreInfoCount   = static_cast<uint32_t>(waitSemaphoreSubmitInfos.size()),
            .pWaitSemaphoreInfos      = waitSemaphoreSubmitInfos.data(),
            .commandBufferInfoCount   = 1,
            .pCommandBufferInfos      = &commandBufferSubmitInfo,
            .signalSemaphoreInfoCount = static_cast<uint32_t>(signalSemaphoreSubmitInfos.size()),
            .pSignalSemaphoreInfos    = signalSemaphoreSubmitInfos.data()
        };
        
        mVulkanDevice.getGraphicsQueue().submit2(submitInfo);
        frame.lastSubmittedValue = graphicsFinishedValue;
        
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
    }   
    mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
#endif

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

const CommandPool& Renderer::getCommandPool() const {
    return mCommandPool;
}

std::vector<std::unique_ptr<Effect>>& Renderer::getEffects() {
    return mEffects;
}

void Renderer::changeImage(const std::filesystem::path& path) {
    ImageConfig imageConfig{
        .image = ImageLoader::loadImageFromPath(path)
    };
    mVulkanDevice.getVkHandle().waitIdle();

    mImage.emplace(mVulkanDevice, mCommandPool, imageConfig);

    for (size_t i{0}; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        const DescriptorImageUpdateConfig imageUpdateConfig{
            .binding = 1,
            .type = vk::DescriptorType::eCombinedImageSampler,
            .image = *mImage,
            .layout = vk::ImageLayout::eShaderReadOnlyOptimal,
            .sampler = mImage->getSampler()
        };
        mDescriptorSets[i].updateImage(imageUpdateConfig);
    }
    _calculateScaling();
    mVulkanDevice.getVkHandle().waitIdle();
}

void Renderer::addEffect(const char* name) {
    mEffects.emplace_back(std::make_unique<Effect>(mVulkanDevice, mComputeDescriptorSetLayout, *mEffectRegistry.getByName(name)));
} 

void Renderer::cleanup() {
    mVulkanDevice.getVkHandle().waitIdle();
    mImGuiSystem.cleanup();
    // mPingPongA.reset();
    // mPingPongB.reset();
    mImage.reset();
    for (auto& effect : mEffects) {
    }
}
