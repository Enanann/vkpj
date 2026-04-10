#include "effect.hpp"
#include "compute_pipeline.hpp"
#include "descriptor_layout.hpp"
#include "device.hpp"

Effect::Effect(std::string_view name, VulkanDevice& device, const std::filesystem::path& path, DescriptorSetLayout& layout, const ComputePipelineConfig& config) 
    : mName{name}
{
    mIsEnabled = true;
    mShader.emplace(device, path);
    mPipeline.emplace(device, *mShader, layout, config);
}

Effect::Effect(VulkanDevice& device, DescriptorSetLayout& layout, const EffectCreateInfo& createInfo) {
    mIsEnabled = true;
    mName = createInfo.name;
    mShader.emplace(device, createInfo.path);
    mPipeline.emplace(device, *mShader, layout, createInfo.config);

    for (const auto& p : createInfo.params) {
        this->addParam(p);
    }
}


Shader& Effect::getShader() {
    return *mShader;
} 

const ComputePipeline& Effect::getPipeline() const {
    return *mPipeline;
}

bool Effect::usePushConstant() {
    return mPipeline->mUsePushConstant;
}

const std::string_view Effect::getName() const {
    return mName;
}

std::vector<EffectParam>& Effect::getParams() {
    return mParams;
} 

std::vector<float>& Effect::getParamsData() {
    return mParamsData;
} 

uint32_t Effect::getFloatParamSize() {
    return static_cast<uint32_t>(mParams.size()) * sizeof(float);
}

void Effect::addParam(EffectParam param) {
    mParams.push_back(param);
    mParamsData.push_back(param.defaultValue);
}

