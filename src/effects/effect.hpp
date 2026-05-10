#pragma once

#include "shader.hpp"
#include "compute_pipeline.hpp"
#include "effect_param.hpp"

#include <cstdint>
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
    uint32_t                    passes{1};
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

    void setSeed(float seed);

    const std::string_view getName() const;
    std::vector<EffectParam>& getParams(); 
    std::vector<float>& getParamsData();
    std::vector<uint8_t> getPackedPushConstants(uint32_t currentPass);
    int getPasses();
private:
    void addParam(EffectParam);

    std::string mName;

    std::optional<Shader> mShader;
    std::optional<ComputePipeline> mPipeline;
    EffectParams             mParams;             
    std::vector<EffectParam> mParamsMetaData;
};
