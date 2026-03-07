#pragma once

#include "platform.hpp"
#include "instance.hpp"
#include "glfw_surface.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include "graphics_pipeline.hpp"
#include "commandpool.hpp"
#include "commandbuffer.hpp"
#include "shader.hpp"
#include "buffer.hpp"

#include <optional>
#include <vulkan/vulkan_raii.hpp>

#include <cstdint>

struct FrameData {
    CommandBuffer       commandBuffer;

    vk::raii::Semaphore timelineSemaphore;
    uint64_t            lastSubmittedValue{0};

    vk::raii::Semaphore imageAvailableSemaphore;
};

class Renderer {
public:
    Renderer(Window&);

    void draw();

    const VulkanDevice& getDevice() const;
private:
    Window&               mWindow;
    Instance              mInstance;
    GLFWSurface           mGLFWSurface;
    VulkanDevice          mVulkanDevice;
    Swapchain             mSwapchain;
    Shader                mShader;
    GraphicsPipeline      mGraphicsPipeline;
    CommandPool           mCommandPool;
    std::optional<Buffer> mVertexBuffer;
    std::optional<Buffer> mIndexBuffer;

    std::vector<FrameData> mFrameDatas;
    uint32_t               mCurrentFrame{0};

    std::vector<vk::raii::Semaphore> mRenderFinishedSemaphores;
};
