#pragma once

#include <vulkan/vulkan_raii.hpp>

class Instance;
class GLFWSurface;

class VulkanDevice {
public:
    VulkanDevice(const Instance&, const GLFWSurface&);

    const vk::raii::PhysicalDevice& getPhysicalDeviceHandle() const;
    const vk::raii::Device& getVkHandle() const;
    const vk::raii::Queue& getGraphicsQueue() const;
    const vk::raii::Queue& getPresentQueue() const;
    uint32_t getGraphicsFamilyIndex() const;
    uint32_t getPresentFamilyIndex() const;

private:
    void pickPhysicalDevice();
    
    void createLogicalDevice();
    
    const Instance& mInstance;
    const GLFWSurface& mSurface;
    
    vk::raii::PhysicalDevice mPhysicalDevice{nullptr};
    vk::raii::Device         mDevice{nullptr};

    uint32_t mGraphicsFamilyIndex;
    vk::raii::Queue mGraphicsQueue{nullptr};
    uint32_t mPresentFamilyIndex;
    vk::raii::Queue mPresentQueue{nullptr};
};
