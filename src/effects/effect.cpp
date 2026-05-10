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
    mParams.passes = createInfo.passes;
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

void Effect::setSeed(float seed) {
    mParams.paramsData[0] = seed;
}

const std::string_view Effect::getName() const {
    return mName;
}

std::vector<float>& Effect::getParamsData() {
    return mParams.paramsData;
}

std::vector<EffectParam>& Effect::getParams() {
    return mParamsMetaData;
} 

std::vector<uint8_t> Effect::getPackedPushConstants(uint32_t currentPass) {
    size_t num_floats{mParams.paramsData.size()};
    size_t total_size{sizeof(int) + num_floats * sizeof(float)};

    std::vector<uint8_t> buffer(total_size, 0);

    // copy passes
    memcpy(buffer.data(), &currentPass, sizeof(int));

    // copy other params
    if (num_floats > 0) {
        memcpy(buffer.data() + sizeof(int), mParams.paramsData.data(), num_floats * sizeof(float));
    }
    
    return buffer;
}

int Effect::getPasses() {
    return mParams.passes;
}

void Effect::addParam(EffectParam param) {
    mParams.paramsData.push_back(param.defaultValue);
    mParamsMetaData.push_back(param);
}

