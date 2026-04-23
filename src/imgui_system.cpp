#include "imgui_system.hpp"

#include "constant.hpp"
#include "platform.hpp"
#include "renderer.hpp"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include <iostream>
#include <utility>

ImGuiSystem::ImGuiSystem(Renderer* renderer) 
    : mRenderer{renderer}
    , mDescriptorPool{mRenderer->getDevice(), {.sizes = {{vk::DescriptorType::eSampler, 1000},
			                                    {vk::DescriptorType::eCombinedImageSampler, 1000},
			                                    {vk::DescriptorType::eSampledImage, 1000},
			                                    {vk::DescriptorType::eStorageImage, 1000},
			                                    {vk::DescriptorType::eUniformTexelBuffer, 1000},
			                                    {vk::DescriptorType::eStorageTexelBuffer, 1000},
			                                    {vk::DescriptorType::eUniformBuffer, 1000},
			                                    {vk::DescriptorType::eUniformBufferDynamic, 1000},
			                                    {vk::DescriptorType::eStorageBufferDynamic, 1000},
			                                    {vk::DescriptorType::eInputAttachment, 1000}}, 
                                    .maxSets = 1000}}
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    // Force consistent background for all window types
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg]       = ImVec4(0.05f, 0.05f, 0.05f, 1.0f); // full opacity
    style.Colors[ImGuiCol_ChildBg]        = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);
    style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);
    style.FontScaleDpi  = 1.36f;
    style.FontScaleMain = 1.2f; 

    ImGui_ImplGlfw_InitForVulkan(mRenderer->getWindow().getGLFWHandle(), true);
    auto _swapchainImageFormat{static_cast<VkFormat>(mRenderer->getSwapchain().getSwapchainImageFormat())};
    ImGui_ImplVulkan_InitInfo initInfo{
        .ApiVersion = VK_VERSION_1_3,
        .Instance   = *mRenderer->getInstance().getVkHandle(),
        .PhysicalDevice = *mRenderer->getDevice().getPhysicalDeviceHandle(),
        .Device = *mRenderer->getDevice().getVkHandle(),
        .QueueFamily = mRenderer->getDevice().getGraphicsFamilyIndex(),
        .Queue = *mRenderer->getDevice().getGraphicsQueue(),
        .DescriptorPool = *mDescriptorPool.getVkHandle(),
        .MinImageCount = MAX_FRAMES_IN_FLIGHT,
        .ImageCount = static_cast<uint32_t>(mRenderer->getSwapchain().getImages().size()),
        // // dynamic rendering parameters
        .PipelineInfoMain = {
            .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
            .PipelineRenderingCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &_swapchainImageFormat
            }
        },
        .UseDynamicRendering = true
    };

    auto res{ImGui_ImplVulkan_Init(&initInfo)};
    if (!res) {
        throw std::runtime_error("Failed to initialize ImGui");
    }
}

void ImGuiSystem::newFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    setupDocking();
}

void ImGuiSystem::render() {
    ImGui::ShowDemoWindow(&mShowDemoWindow);

    if(ImGui::Begin("File browser")) {
        // open file dialog when user clicks this button
        if(ImGui::Button("open file dialog"))
            mFilebrowser.Open();

        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    }
    ImGui::End();
        
    mFilebrowser.Display();
    
    if(mFilebrowser.HasSelected()) {
        std::cout << "Selected filename" << mFilebrowser.GetSelected().string() << std::endl;
        mRenderer->changeImage(mFilebrowser.GetSelected());
        mFilebrowser.ClearSelected();
    }

    if (ImGui::Begin("Effects")) {
        std::optional<size_t> effectToRemove{std::nullopt};
        // Drag and drop to reorder effect
        for (size_t i{0}; i < mRenderer->getEffects().size(); ++i) {
            auto& e = mRenderer->getEffects()[i];
    
            ImGui::PushID(static_cast<int>(i));
    
            // Open as default
            auto nodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;
            bool nodeOpen = ImGui::TreeNodeEx(e->getName().data(), nodeFlags);
    
            // Drag and drop (uses the most recent widget)
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                ImGui::SetDragDropPayload("EFFECT_CELL", &i, sizeof(size_t));
                ImGui::Text("Dragging: %s", e->getName().data());
                ImGui::EndDragDropSource();
            }
    
            if (ImGui::BeginDragDropTarget()) {
                // Accept if tpye is "EFFECT_CELL"
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EFFECT_CELL")) {
                    // Get the drag source index
                    IM_ASSERT(payload->DataSize == sizeof(size_t));
                    size_t payload_n = *(const size_t*)payload->Data; 
                    size_t target    = i;
    
                    /*
                    std::rotate(first, mid, last);
                    Parameters:
                        first: Iterator to the first element in the range.
                        mid: Iterator to the element that becomes the new first element.
                        last: Iterator to the theoretical element just after the last element in the range.
                    */
                    if (payload_n < target) {
                        std::rotate(mRenderer->getEffects().begin() + payload_n, 
                                    mRenderer->getEffects().begin() + payload_n + 1, 
                                    mRenderer->getEffects().begin() + target + 1);
                    } else {
                        std::rotate(mRenderer->getEffects().begin() + target, 
                                    mRenderer->getEffects().begin() + payload_n, 
                                    mRenderer->getEffects().begin() + payload_n + 1);
                    }
                    // std::swap(mRenderer->getEffects()[payload_n], mRenderer->getEffects()[i]);
                }
                ImGui::EndDragDropTarget();
            }

            // Remove effect
            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 25.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
            float _size = ImGui::GetFrameHeight();
            if (ImGui::Button("X", ImVec2(_size, _size))) {
                effectToRemove = i;
            }
            ImGui::PopStyleColor(1);
    
            // Effect params
            if (nodeOpen) {
                ImGui::Checkbox("Enable", &e->mIsEnabled);
                if (e->getPipeline().mUsePushConstant) {
                    for (size_t j{0}; j < e->getParams().size(); ++j) {
                        const auto& p{e->getParams()[j]};
                        ImGui::SliderFloat(p.displayName.c_str(), &e->getParamsData()[j], p.min, p.max);
                    }
                }
                ImGui::TreePop();
            }
    
            ImGui::PopID();
        }
        if (effectToRemove.has_value()) {
            mRenderer->getDevice().getVkHandle().waitIdle();
            mRenderer->getEffects().erase(mRenderer->getEffects().begin() + effectToRemove.value());
        }

        if (ImGui::Button("Add effect")) {
            ImGui::OpenPopup("effect_selection");
        }
        if (ImGui::BeginPopup("effect_selection")) {
            if (ImGui::Selectable("Grayscale")) {
                mRenderer->addEffect("Grayscale");
            }
            if (ImGui::Selectable("Vignette")) {
                mRenderer->addEffect("Vignette");
            }
            if (ImGui::Selectable("HSL")) {
                mRenderer->addEffect("HSL");
            }
            if (ImGui::Selectable("Sepia")) {
                mRenderer->addEffect("Sepia");
            }
            if (ImGui::Selectable("Lens Distortion")) {
                mRenderer->addEffect("Lens Distortion");
            }
            if (ImGui::Selectable("Brightness & Contrast")) {
                mRenderer->addEffect("Brightness and Contrast");
            }
            if (ImGui::Selectable("Exposure")) {
                mRenderer->addEffect("Exposure");
            }
            if (ImGui::Selectable("Sobel")) {
                mRenderer->addEffect("Sobel");
            }
            if (ImGui::Selectable("Gaussian Noise")) {
                mRenderer->addEffect("Gaussian Noise");
            }
            if (ImGui::Selectable("Salt and Pepper")) {
                mRenderer->addEffect("Salt and Pepper");
            }
            if (ImGui::Selectable("Pseudo-Median filter")) {
                mRenderer->addEffect("Pseudo-Median filter");
            }
            if (ImGui::Selectable("Median filter")) {
                mRenderer->addEffect("Median filter");
            }

            ImGui::EndPopup();
        }
    }
    ImGui::End();

    ImGui::Render();
}

void ImGuiSystem::setupDocking() {
    static bool dockspaceOpen = true;
    static bool opt_fullscreen_persistant = true;
    bool opt_fullscreen = opt_fullscreen_persistant;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
    // window_flags |= ImGuiWindowFlags_MenuBar;
    if (opt_fullscreen) {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
    ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) {
                glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // // Call user-defined UI
    // if (uiCallback) {
    //     uiCallback();
    // }

    ImGui::End();
}

void ImGuiSystem::cleanup() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
