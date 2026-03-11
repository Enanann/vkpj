#pragma once

#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>

#include <vector>

class VulkanDevice;
class GLFWSurface;
class Window;

class Swapchain {
public:
    Swapchain(const VulkanDevice&, const GLFWSurface&, Window&);
    void createImageViews();
    void recreate();
    void cleanup();
    vk::Format& getFormat();
    const std::vector<vk::Image>& getImages() const;
    const std::vector<vk::raii::ImageView>& getImageViews() const;
    const vk::Extent2D& getExtent() const;
    const vk::Format& getSwapchainImageFormat() const;

    const vk::raii::SwapchainKHR& getVkHandle() const;

private:
    vk::SurfaceFormatKHR chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR   choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D         chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

    void create();
    vk::raii::ImageView createImageView(vk::Image&, vk::Format, vk::ImageAspectFlags);

    const VulkanDevice& mVulkanDevice;
    const GLFWSurface& mSurface;
    Window& mWindow;

    vk::raii::SwapchainKHR mSwapchain{nullptr};
    std::vector<vk::Image> mSwapchainImages;
    std::vector<vk::raii::ImageView> mSwapchainImageViews;

    vk::Format mSwapchainImageFormat{vk::Format::eUndefined};
    vk::Extent2D mSwapchainExtent;
};
