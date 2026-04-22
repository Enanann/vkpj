#include "effect_registry.hpp"
#include "effect.hpp"

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
        .min          = -1.0f,
        .max          = 1.0f 
    });
    hsl.params.push_back({
        .displayName  = "Lightness",
        .defaultValue = 0.0f,
        .min          = -1.0f,
        .max          = 1.0f 
    });

    EffectCreateInfo sepia("Sepia", "build/src/shaders/sepia.spv", {.usePushConstant = false, .pushConstantSize = 1});

    EffectCreateInfo lens_distortion("Lens Distortion", "build/src/shaders/lens_distortion.spv", {.usePushConstant = true, .pushConstantSize = 16});
    lens_distortion.params.push_back({
        .displayName  = "k1",
        .defaultValue = 0.0f,
        .min          = -1.0f,
        .max          = 2.0f
    });
    lens_distortion.params.push_back({
        .displayName  = "k2",
        .defaultValue = 0.0f,
        .min          = 0.0f,
        .max          = 2.0f
    });
    lens_distortion.params.push_back({
        .displayName  = "k3",
        .defaultValue = 0.0f,
        .min          = 0.0f,
        .max          = 2.0f
    });
    lens_distortion.params.push_back({
        .displayName  = "Zoom",
        .defaultValue = 1.0f,
        .min          = 1.0f,
        .max          = 3.0f
    });

    EffectCreateInfo bri_con("Brightness and Contrast", "build/src/shaders/brightness_contrast.spv", {.usePushConstant = true, .pushConstantSize = 8});
    bri_con.params.push_back({
        .displayName  = "Brightness",
        .defaultValue = 0.0f,
        .min          = -1.0f,
        .max          = 1.0f
    });
    bri_con.params.push_back({
        .displayName  = "Contrast",
        .defaultValue = 1.0f,
        .min          = 0.0f,
        .max          = 5.0f
    });

    EffectCreateInfo exposure("Exposure", "build/src/shaders/exposure.spv", {.usePushConstant = true, .pushConstantSize = 8});
    exposure.params.push_back({
        .displayName  = "Black level",
        .defaultValue = 0.0f,
        .min          = -0.1f,
        .max          = 0.1f
    });
    exposure.params.push_back({
        .displayName  = "Exposure",
        .defaultValue = 0.0f,
        .min          = -10.0f,
        .max          = 10.0f
    });

    EffectCreateInfo sobel("Sobel", "build/src/shaders/sobel.spv", {.usePushConstant = false, .pushConstantSize = 1});

    EffectCreateInfo gaussian_noise("Gaussian Noise", "build/src/shaders/gaussian_noise.spv", {.usePushConstant = true, .pushConstantSize = 8});
    gaussian_noise.params.push_back({
        .displayName  = "Seed",
        .defaultValue = 0.0f,
        .min          = 0.0f,
        .max          = 0.0f
    });
    gaussian_noise.params.push_back({
        .displayName  = "Intensity",
        .defaultValue = 0.1f,
        .min          = 0.0f,
        .max          = 1.0f
    });

    EffectCreateInfo salt_pepper("Salt and Pepper", "build/src/shaders/salt_and_pepper.spv", {.usePushConstant = true, .pushConstantSize = 8});
    salt_pepper.params.push_back({
        .displayName  = "Seed",
        .defaultValue = 0.0f,
        .min          = 0.0f,
        .max          = 0.0f
    });
    salt_pepper.params.push_back({
        .displayName  = "Probability",
        .defaultValue = 0.05f,
        .min          = 0.0f,
        .max          = 1.0f
    });

    mEffectCreateInfos.push_back(grayscale);
    mEffectCreateInfos.push_back(vignette);
    mEffectCreateInfos.push_back(hsl);
    mEffectCreateInfos.push_back(sepia);
    mEffectCreateInfos.push_back(lens_distortion);
    mEffectCreateInfos.push_back(bri_con);
    mEffectCreateInfos.push_back(exposure);
    mEffectCreateInfos.push_back(sobel);
    mEffectCreateInfos.push_back(gaussian_noise);
    mEffectCreateInfos.push_back(salt_pepper);
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
