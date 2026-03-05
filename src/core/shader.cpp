#include "shader.hpp"
#include "read_binary.hpp"

#include "device.hpp"
#include <vulkan/vulkan_raii.hpp>

Shader::Shader(const VulkanDevice& vulkanDevice, const std::string& filepath) : mVulkanDevice{vulkanDevice} {
    mShaderModule = createShaderModule(BinaryReader::readFile((filepath)));

    vk::PipelineShaderStageCreateInfo vertexShaderInfo {
        .stage  = vk::ShaderStageFlagBits::eVertex,
        .module = mShaderModule,
        .pName  = "vertMain"
    };
    vk::PipelineShaderStageCreateInfo fragmentShaderInfo {
        .stage  = vk::ShaderStageFlagBits::eFragment,
        .module = mShaderModule,
        .pName  = "fragMain"
    };
    vk::PipelineShaderStageCreateInfo computeShaderInfo {
        .stage  = vk::ShaderStageFlagBits::eCompute,
        .module = mShaderModule,
        .pName  = "compMain"
    };

    mShaderStages.clear();
    mShaderStages = {vertexShaderInfo, fragmentShaderInfo, computeShaderInfo};
}

const vk::raii::ShaderModule& Shader::getShaderModule() {
    return mShaderModule;
}

std::vector<vk::PipelineShaderStageCreateInfo>& Shader::getShaderStages() {
    return mShaderStages;
}

[[nodiscard]] vk::raii::ShaderModule Shader::createShaderModule(const std::vector<char>& shaderCode) {
    vk::ShaderModuleCreateInfo shaderModuleCreateInfo{
        .codeSize = shaderCode.size() * sizeof(char),
        .pCode    = reinterpret_cast<const uint32_t*>(shaderCode.data())
    };
    vk::raii::ShaderModule shaderModule(mVulkanDevice.getVkHandle(), shaderModuleCreateInfo);
    return shaderModule;
}
