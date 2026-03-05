#include "device.hpp"
#include "instance.hpp"
#include "glfw_surface.hpp"

#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>

#include <cstdint>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <set>

std::vector<const char*> deviceExtensions = {
    vk::KHRSwapchainExtensionName,
    vk::KHRSpirv14ExtensionName,
    vk::KHRSynchronization2ExtensionName,
    vk::KHRCreateRenderpass2ExtensionName
};

VulkanDevice::VulkanDevice(const Instance& instance, const GLFWSurface& surface) : mInstance{instance}, mSurface{surface} {
    pickPhysicalDevice();
    createLogicalDevice();
}

const vk::raii::PhysicalDevice& VulkanDevice::getPhysicalDeviceHandle() const {
    return mPhysicalDevice;
}

const vk::raii::Device& VulkanDevice::getVkHandle() const {
    return mDevice;
}

const vk::raii::Queue& VulkanDevice::getGraphicsQueue() const {
    return mGraphicsQueue;
}

const vk::raii::Queue& VulkanDevice::getPresentQueue() const {
    return mPresentQueue;
}


uint32_t VulkanDevice::getGraphicsFamilyIndex() const {
    return mGraphicsFamilyIndex;
}

uint32_t VulkanDevice::getPresentFamilyIndex() const {
    return mPresentFamilyIndex;
}


void VulkanDevice::pickPhysicalDevice() {
    auto availablePhysicalDevices{mInstance.getVkHandle().enumeratePhysicalDevices()};

    if (availablePhysicalDevices.empty()) {
        throw std::runtime_error("Can not find any GPUs with Vulkan support");
    }

    // Check for suitable graphics card
    auto devIter{std::ranges::find_if(availablePhysicalDevices, [&](const vk::raii::PhysicalDevice& physicalDevice) {
        // Check support for Vulkan version 1.3 or newer
        bool supportVulkan13{physicalDevice.getProperties().apiVersion >= VK_VERSION_1_3};

        // Find the first queue family that support graphics
        auto queueFamilies{physicalDevice.getQueueFamilyProperties()};
        auto qfIter{std::ranges::find_if(queueFamilies, [](const vk::QueueFamilyProperties& qf) {
            return (qf.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0);
        })}; 
        bool supportGraphics{qfIter != queueFamilies.end()};

        // Check support for all the required extensions
        auto availableExtensions{physicalDevice.enumerateDeviceExtensionProperties()};
        auto supportAllRequiredExtensions{std::ranges::all_of(deviceExtensions, [availableExtensions](const auto& deviceExtension) {
            return std::ranges::any_of(availableExtensions, [deviceExtension](const auto& availableExtension) {
                return strcmp(deviceExtension, availableExtension.extensionName) == 0;
            });
        })};

        // Check support for all the required features
        // template getFeatures2<X, Y, Z,...> return a StructureChain<X, Y, Z,...>
        // template get<X> return struct X
        auto supportedFeatures = physicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2, 
                                                                    vk::PhysicalDeviceVulkan11Features, 
                                                                    vk::PhysicalDeviceVulkan13Features,
                                                                    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT,
                                                                    vk::PhysicalDeviceTimelineSemaphoreFeatures>();
        bool supportAllRequiredFeatures = supportedFeatures.template get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy &&
                                          supportedFeatures.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
                                          supportedFeatures.template get<vk::PhysicalDeviceVulkan13Features>().synchronization2 &&
                                          supportedFeatures.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
                                          supportedFeatures.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState &&
                                          supportedFeatures.template get<vk::PhysicalDeviceTimelineSemaphoreFeatures>().timelineSemaphore;

        return supportVulkan13 && supportGraphics && supportAllRequiredExtensions && supportAllRequiredFeatures;
    })};

    if (devIter == availablePhysicalDevices.end()) {
        throw std::runtime_error("Failed to find suitable GPU");
    } else {
        mPhysicalDevice = *devIter;
        // std::cout << "Picked " << *devIter->getProperties().deviceName << std::endl;
        std::cout << "Picked " + std::string(mPhysicalDevice.getProperties().deviceName) << '\n';
    }
}

void VulkanDevice::createLogicalDevice() {
    auto queueFamilyProperties{mPhysicalDevice.getQueueFamilyProperties()};
    
    uint32_t graphicsIndex{static_cast<uint32_t>(queueFamilyProperties.size())};
    uint32_t presentIndex{graphicsIndex};
    for (size_t i{0}; i < queueFamilyProperties.size(); ++i) {
        // graphics + compute
        auto supportGraphics{
            (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0) &&
            (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eCompute) != static_cast<vk::QueueFlags>(0)
        };

        auto supportPresent{mPhysicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), mSurface.getVkHandle())};

        if (supportGraphics && supportPresent) {
            graphicsIndex = static_cast<uint32_t>(i);
            presentIndex = graphicsIndex;
            break;
        }

        if (supportGraphics && (graphicsIndex == static_cast<uint32_t>(queueFamilyProperties.size()))) {
            graphicsIndex = static_cast<uint32_t>(i);
        }
        if (supportPresent && (presentIndex == static_cast<uint32_t>(queueFamilyProperties.size()))) {
            presentIndex = static_cast<uint32_t>(i);
        }
    }

    // if can't find anything
    if ((graphicsIndex == queueFamilyProperties.size()) || (presentIndex == queueFamilyProperties.size())) {
        throw std::runtime_error("Could not find a queue for graphics or present");
    }

    // DeviceQueueCreateInfo
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamily = {graphicsIndex, presentIndex};
    mGraphicsFamilyIndex = graphicsIndex;
    mPresentFamilyIndex = presentIndex;
    float queuePriority{1.0f};
    for (auto qf : uniqueQueueFamily) {
        queueCreateInfos.push_back({.queueFamilyIndex = qf, .queueCount = 1, .pQueuePriorities = &queuePriority});
    }
    
    // Structure chain for features
    vk::StructureChain<vk::PhysicalDeviceFeatures2,
                        vk::PhysicalDeviceVulkan11Features,
                        vk::PhysicalDeviceVulkan13Features,
                        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT,
                        vk::PhysicalDeviceTimelineSemaphoreFeatures> featureChain = {
        {.features = {.samplerAnisotropy = true}},
        {.shaderDrawParameters = true},
        {.synchronization2 = true, .dynamicRendering = true},
        {.extendedDynamicState = true},
        {.timelineSemaphore = true}
    };

    vk::DeviceCreateInfo deviceCreateInfo{
        .pNext                   = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos       = queueCreateInfos.data(),
        .enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data()
    };

    mDevice        = vk::raii::Device(mPhysicalDevice, deviceCreateInfo);
    mGraphicsQueue = vk::raii::Queue(mDevice, mGraphicsFamilyIndex, 0);
    mPresentQueue  = vk::raii::Queue(mDevice, mPresentFamilyIndex, 0);
}

