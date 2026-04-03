#pragma once

#include "compute_pipeline.hpp"
#include "descriptor_layout.hpp"
#include "descriptor_pool.hpp"
#include "descriptor_set.hpp"
#include "image.hpp"
#include "imgui_system.hpp"
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
#include "effect.hpp"

#include <filesystem>
#include <optional>
#include <vector>
#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>

#include <cstdint>

struct FrameData {
    CommandBuffer        commandBuffer;
    ComputeCommandBuffer computeCommandBuffer;

    std::optional<Image> ping;
    std::optional<Image> pong;

    vk::raii::Semaphore timelineSemaphore;
    uint64_t            lastSubmittedValue{0};

    vk::raii::Semaphore imageAvailableSemaphore;
};

class Renderer {
public:
    Renderer(Window&);

    void drawImGui();
    void draw();
    void setPan(glm::vec2&);
    void setZoom(float);

    void changeImage(const std::filesystem::path&);

    const Window&       getWindow() const;
    const Instance&     getInstance() const;
    const VulkanDevice& getDevice() const;
    const Swapchain&    getSwapchain() const;
    const CommandPool&  getCommandPool() const;

    void cleanup();
private:
    void _calculateScaling();

    Window&               mWindow;
    Instance              mInstance;
    GLFWSurface           mGLFWSurface;
    VulkanDevice          mVulkanDevice;
    Swapchain             mSwapchain;
    Shader                mShader;
    std::vector<Effect>   mEffects;
    DescriptorSetLayout   mDescriptorSetLayout;
    DescriptorSetLayout   mComputeDescriptorSetLayout;
    GraphicsPipeline      mGraphicsPipeline;
    CommandPool           mCommandPool;
    std::optional<Buffer> mVertexBuffer;
    std::optional<Buffer> mIndexBuffer;
    std::vector<std::optional<Buffer>> mUniformBuffers;
    DescriptorPool             mDescriptorPool;
    std::vector<DescriptorSet> mDescriptorSets;
    std::vector<DescriptorSet> mComputeDescriptorSetsInit;
    std::vector<DescriptorSet> mComputeDescriptorSetsAtoB;
    std::vector<DescriptorSet> mComputeDescriptorSetsBtoA;
    std::optional<Image>       mImage;
    std::vector<FrameData>     mFrameDatas;
    uint32_t                   mCurrentFrame{0};

    glm::vec2 mScale{1.0f, 1.0f};
    glm::vec2 mPan{0.0f, 0.0f};
    float     mZoom{45.0f};

    ImGuiSystem mImGuiSystem;

    std::vector<vk::raii::Semaphore> mRenderFinishedSemaphores;
};
