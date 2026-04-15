#include "effect_registry.hpp"

#include <algorithm>
// #include <stdexcept>
// #include <format>

EffectRegistry::EffectRegistry() {
    EffectCreateInfo grayscale{"Grayscale", "build/src/shaders/grayscale.spv", {.usePushConstant = false, .pushConstantSize = 1}};

    EffectCreateInfo vignette{"Vignette", "build/src/shaders/vignette.spv", {.usePushConstant = true, .pushConstantSize = 8}};
    vignette.params.push_back({
        .displayName  = "Radius",
        .defaultValue = 0.8,
        .min = 0.0,
        .max = 1.0
    });
    vignette.params.push_back({
        .displayName  = "Intensity",
        .defaultValue = 1.0,
        .min = 0.0,
        .max = 3.0
    });

    EffectCreateInfo hsl{"HSL", "build/src/shaders/hsl.spv", {.usePushConstant = true, .pushConstantSize = 12}};
    hsl.params.push_back({
        .displayName  = "Hue",
        .defaultValue = 0.0f,
        .min          = 0.0f,
        .max          = 1.0f 
    });
    hsl.params.push_back({
        .displayName  = "Saturation",
        .defaultValue = 0.0f,
        .min          = 0.0f,
        .max          = 1.0f 
    });
    hsl.params.push_back({
        .displayName  = "Lightness",
        .defaultValue = 0.0f,
        .min          = 0.0f,
        .max          = 1.0f 
    });

    mEffectCreateInfos.push_back(grayscale);
    mEffectCreateInfos.push_back(vignette);
    mEffectCreateInfos.push_back(hsl);
}

// const std::vector<Effect>& EffectRegistry::getEffects() const noexcept {
//     return mEffects;
// } 

const EffectCreateInfo* EffectRegistry::getByName(std::string_view name) const noexcept {
    auto efIt{std::ranges::find_if(mEffectCreateInfos, [name](const EffectCreateInfo& e) {
        return e.name == name;
    })};
    if (efIt == mEffectCreateInfos.end()) {
        // throw std::runtime_error(std::format("No effect named {} in registry!", name));
        return nullptr;
    }
    return &*efIt;
}
