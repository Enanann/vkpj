#include "instance.hpp"

#include <GLFW/glfw3.h>

#include <string>
#include <cstdint>
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

const std::vector<char const*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

Instance::Instance() {
    constexpr vk::ApplicationInfo appInfo{
        .pApplicationName   = "App",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName        = "No Engine",
        .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion         = vk::ApiVersion14
    };

    // Get the required instance extensions from GLFW
    uint32_t glfwExtensionCount{0};
    auto glfwExtensions{glfwGetRequiredInstanceExtensions(&glfwExtensionCount)};

    // Check if the required instance extensions above are supported by the Vulkan implementation
    auto extensionProperties{mContext.enumerateInstanceExtensionProperties()};
    for (uint32_t i{0}; i < glfwExtensionCount; ++i) {
        if (std::ranges::none_of(extensionProperties,
                                [glfwExtension = glfwExtensions[i]](auto const& extensionProperty) {
                                    return strcmp(extensionProperty.extensionName, glfwExtension) == 0;
                                })) 
        {
            throw std::runtime_error("Required GLFW extension not supported: " + std::string(glfwExtensions[i]));
        }
    }

    // Get the required layers
    std::vector<char const*> requiredLayers{};
    if (enableValidationLayers) {
        requiredLayers.assign(validationLayers.begin(), validationLayers.end());
    }
    // Check if the required layers are supported by the Vulkan implementation
    auto layerProperties{mContext.enumerateInstanceLayerProperties()};
    for (const auto& layer : requiredLayers) {
        if (std::ranges::none_of(layerProperties, [requiredLayer = layer](const auto& layerProperty) {
            return strcmp(requiredLayer, layerProperty.layerName) == 0;
        }))
        {
            throw std::runtime_error("Required layer not supported: " + std::string(layer));
        }
    }

    vk::InstanceCreateInfo instanceCreateInfo{
        .pApplicationInfo        = &appInfo,
        .enabledLayerCount       = static_cast<uint32_t>(requiredLayers.size()),
        .ppEnabledLayerNames     = requiredLayers.data(),
        .enabledExtensionCount   = glfwExtensionCount,
        .ppEnabledExtensionNames = glfwExtensions
    };

    mInstance = vk::raii::Instance(mContext, instanceCreateInfo);
}


const vk::raii::Instance& Instance::getVkHandle() const {
    return mInstance;
}
