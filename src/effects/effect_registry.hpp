#pragma once

#include "effect.hpp"

#include <string_view>
#include <vector>

class VulkanDevice;
class DescriptorSetLayout;

class EffectRegistry {
public:
    EffectRegistry();

    // [[nodiscard]] const std::vector<Effect>& getEffects() const noexcept;
    [[nodiscard]] const EffectCreateInfo* getByName(std::string_view) const noexcept;

private:
    std::vector<EffectCreateInfo> mEffectCreateInfos;
};
