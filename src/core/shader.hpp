#pragma once

#include "device.hpp"

#include <vulkan/vulkan_raii.hpp>

// #include <filesystem>
#include <vector>

class VulkanDevice;

class Shader {
public:
    // Shader(const VulkanDevice&, const std::filesystem::path&);
    Shader(const VulkanDevice&, const std::string&);
    
    const vk::raii::ShaderModule& getShaderModule();
    std::vector<vk::PipelineShaderStageCreateInfo>& getShaderStages();

private:
    [[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char>& shaderCode);

    const VulkanDevice& mVulkanDevice;

    vk::raii::ShaderModule mShaderModule{nullptr};
    std::vector<vk::PipelineShaderStageCreateInfo> mShaderStages;
};
