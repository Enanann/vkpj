#include "swapchain.hpp"

#include "device.hpp"
#include "platform.hpp"
#include "glfw_surface.hpp"
#include "vulkan/vulkan.hpp"
#include <algorithm>
#include <limits>
#include <vulkan/vulkan_raii.hpp>

Swapchain::Swapchain(const VulkanDevice& device, const GLFWSurface& surface, Window& window) 
    : mVulkanDevice{device}
    , mSurface{surface}
    , mWindow{window} 
{
    create();
    createImageViews();
}

vk::raii::ImageView Swapchain::createImageView(vk::Image& image, vk::Format format, vk::ImageAspectFlags aspeckMask) {
    vk::ImageViewCreateInfo imageViewCreateInfo {
        .image    = image,
        .viewType = vk::ImageViewType::e2D,
        .format   = format,
        .subresourceRange = {
            .aspectMask     = aspeckMask,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1
        }
    };
    return vk::raii::ImageView(mVulkanDevice.getVkHandle(), imageViewCreateInfo);
}

void Swapchain::recreate() {
    int width{0};
	int height{0};
	mWindow.getFrameBufferSize(&width, &height);
	while (width == 0 || height == 0) {
		mWindow.getFrameBufferSize(&width, &height);
		mWindow.wait();
	}

    mVulkanDevice.getVkHandle().waitIdle();
    cleanup();
    create();
    createImageViews();
}

void Swapchain::cleanup() {
    mSwapchainImageViews.clear();
    mSwapchain = nullptr;
    // vk::raii::SwapchainKHR disables copy construction and copy assignment.
    // Assigning nullptr creates a temporary empty SwapchainKHR.
    // Move-assignment swaps the internal handles, transferring ownership of the old swapchain
    // to the temporary, which is then destroyed at the end of the full-expression.
}

vk::raii::SwapchainKHR& Swapchain::getVkHandle() {
    return mSwapchain;
}

/*Prefer SRGB, else choose the first one available*/
vk::SurfaceFormatKHR Swapchain::chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb
            && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

/*Choose VK_PRESENT_MODE_MAILBOX_KHR if possible, else choose VK_PRESENT_MODE_FIFO_KHR*/
vk::PresentModeKHR Swapchain::choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        }
    }
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Swapchain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    int width{};
    int height{};
    mWindow.getFrameBufferSize(&width, &height);

    return {
        .width  = std::clamp<uint32_t>(width,
                                      capabilities.minImageExtent.width,
                                      capabilities.maxImageExtent.width),
        .height = std::clamp<uint32_t>(height,
                                       capabilities.minImageExtent.height,
                                       capabilities.maxImageExtent.height)
    };
}

void Swapchain::create() {
    std::vector<vk::SurfaceFormatKHR> availableFormats{mVulkanDevice.getPhysicalDeviceHandle().getSurfaceFormatsKHR(mSurface.getVkHandle())};
    std::vector<vk::PresentModeKHR>   availablePresentModes{mVulkanDevice.getPhysicalDeviceHandle().getSurfacePresentModesKHR(mSurface.getVkHandle())};
    vk::SurfaceCapabilitiesKHR        surfaceCapabilities{mVulkanDevice.getPhysicalDeviceHandle().getSurfaceCapabilitiesKHR(mSurface.getVkHandle())};

    uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
    // maxImageCount = 0 meant that there're unlimited images
    imageCount = (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) 
                    ? surfaceCapabilities.maxImageCount 
                    : imageCount;

    // Create the swapchain
    vk::SurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(availableFormats);
    vk::Extent2D         extent = chooseSwapExtent(surfaceCapabilities);

    vk::SwapchainCreateInfoKHR swapchainCreateInfo{
        .flags            = vk::SwapchainCreateFlagsKHR(),
        .surface          = mSurface.getVkHandle(),
        .minImageCount    = imageCount,
        .imageFormat      = surfaceFormat.format,
        .imageColorSpace  = surfaceFormat.colorSpace,
        .imageExtent      = extent,
        .imageArrayLayers = 1,
        .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
        // .imageSharingMode
        .preTransform     = surfaceCapabilities.currentTransform,
        .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode      = choosePresentMode(availablePresentModes),
        .clipped          = true,
        .oldSwapchain     = nullptr
    };

    uint32_t queueFamilyIndices[] = {mVulkanDevice.getGraphicsFamilyIndex(), mVulkanDevice.getPresentFamilyIndex()};
    if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
        swapchainCreateInfo.imageSharingMode      = vk::SharingMode::eConcurrent;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices   = queueFamilyIndices;
    } else {
        swapchainCreateInfo.imageSharingMode      = vk::SharingMode::eExclusive;
        swapchainCreateInfo.queueFamilyIndexCount = 0;       // optional
        swapchainCreateInfo.pQueueFamilyIndices   = nullptr; // optional
    }

    mSwapchain = vk::raii::SwapchainKHR(mVulkanDevice.getVkHandle(), swapchainCreateInfo);
    mSwapchainImages = mSwapchain.getImages();
    mSwapchainImageFormat = surfaceFormat.format;
    mSwapchainExtent = extent;
}

void Swapchain::createImageViews() {
    mSwapchainImageViews.clear();

    for (auto& image : mSwapchainImages) {
        mSwapchainImageViews.emplace_back(createImageView(image, mSwapchainImageFormat, vk::ImageAspectFlagBits::eColor));
    }
}

vk::Format& Swapchain::getFormat() {
    return mSwapchainImageFormat;
}

std::vector<vk::Image>& Swapchain::getImages() {
    return mSwapchainImages;
}

std::vector<vk::raii::ImageView>& Swapchain::getImageViews() {
    return mSwapchainImageViews;
}

vk::Extent2D& Swapchain::getExtent() {
    return mSwapchainExtent;
}



