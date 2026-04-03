#pragma once

#include "shader.hpp"
#include "compute_pipeline.hpp"
#include "effect_param.hpp"

#include <filesystem>
#include <optional>
#include <vector>
#include <string>

class VulkanDevice;
class DescriptorSetLayout;

class Effect {
public:
    Effect(VulkanDevice&, const std::filesystem::path&, DescriptorSetLayout&);

    Shader& getShader();
    ComputePipeline& getPipeline();
private:
    std::string mName;

    std::optional<Shader> mShader;
    std::optional<ComputePipeline> mPipeline;
    std::vector<EffectParam> mParams;
};
