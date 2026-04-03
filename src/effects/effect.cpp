#include "effect.hpp"
#include "compute_pipeline.hpp"
#include "descriptor_layout.hpp"
#include "device.hpp"

Effect::Effect(VulkanDevice& device, const std::filesystem::path& path, DescriptorSetLayout& layout) {
    mShader.emplace(device, path);
    mPipeline.emplace(device, *mShader, layout);
}

Shader& Effect::getShader() {
    return *mShader;
} 

ComputePipeline& Effect::getPipeline() {
    return *mPipeline;
}
