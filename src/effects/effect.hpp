#pragma once

#include "shader.hpp"
#include "compute_pipeline.hpp"
#include "effect_param.hpp"

#include <filesystem>
#include <optional>
#include <string_view>
#include <vector>
#include <string>

class VulkanDevice;
class DescriptorSetLayout;

struct EffectCreateInfo {
    std::string                 name;
    const std::filesystem::path path;
    ComputePipelineConfig       config;
    std::vector<EffectParam>    params;
};

class Effect {
public:
    bool mIsEnabled;

    Effect(std::string_view, VulkanDevice&, const std::filesystem::path&, DescriptorSetLayout&, const ComputePipelineConfig&);
    Effect(VulkanDevice&, DescriptorSetLayout&, const EffectCreateInfo&);

    Shader& getShader();
    const ComputePipeline& getPipeline() const;
    bool usePushConstant();

    const std::string_view getName() const;
    std::vector<EffectParam>& getParams(); 
    std::vector<float>& getParamsData(); 
    uint32_t getFloatParamSize();
private:
    void addParam(EffectParam);

    std::string mName;

    std::optional<Shader> mShader;
    std::optional<ComputePipeline> mPipeline;
    std::vector<EffectParam> mParams;
    std::vector<float>       mParamsData;
};
