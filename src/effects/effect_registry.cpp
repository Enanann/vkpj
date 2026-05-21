#include "effect_registry.hpp"
#include "effect.hpp"

#include <algorithm>
#include <numbers>
// #include <stdexcept>
// #include <format>

EffectRegistry::EffectRegistry() {
    EffectCreateInfo grayscale{"Grayscale", "build/src/shaders/grayscale.spv", {.usePushConstant = false, .pushConstantSize = 1}};

    EffectCreateInfo vignette{"Vignette", "build/src/shaders/vignette.spv", {.usePushConstant = true, .pushConstantSize = 12}};
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

    EffectCreateInfo hsl{"HSL", "build/src/shaders/hsl.spv", {.usePushConstant = true, .pushConstantSize = 16}};
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

    EffectCreateInfo lens_distortion("Lens Distortion", "build/src/shaders/lens_distortion.spv", {.usePushConstant = true, .pushConstantSize = 20});
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

    EffectCreateInfo bri_con("Brightness and Contrast", "build/src/shaders/brightness_contrast.spv", {.usePushConstant = true, .pushConstantSize = 12});
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

    EffectCreateInfo exposure("Exposure", "build/src/shaders/exposure.spv", {.usePushConstant = true, .pushConstantSize = 12});
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

    EffectCreateInfo gaussian_noise("Gaussian Noise", "build/src/shaders/gaussian_noise.spv", {.usePushConstant = true, .pushConstantSize = 12});
    gaussian_noise.params.push_back({
        .visible      = false,
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

    EffectCreateInfo salt_pepper("Salt and Pepper", "build/src/shaders/salt_and_pepper.spv", {.usePushConstant = true, .pushConstantSize = 12});
    salt_pepper.params.push_back({
        .visible      = false,
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

    EffectCreateInfo pseudo_median_filter("Pseudo-Median filter", "build/src/shaders/pseudo_median_filter.spv", {.usePushConstant = false, .pushConstantSize = 1});

    EffectCreateInfo median_filter("Median filter", "build/src/shaders/median_filter.spv", {.usePushConstant = false, .pushConstantSize = 1});
    
    EffectCreateInfo chromatic_aberration("Chromatic aberration", "build/src/shaders/chromatic_aberration.spv", {.usePushConstant = true, .pushConstantSize = 20});
    chromatic_aberration.params.push_back({
        .displayName  = "Red offset",
        .defaultValue = 0.5f,
        .min          = 0.0f,
        .max          = 1.0f
    });
    chromatic_aberration.params.push_back({
        .displayName  = "Green offset",
        .defaultValue = 0.7f,
        .min          = 0.0f,
        .max          = 1.0f
    });
    chromatic_aberration.params.push_back({
        .displayName  = "Blue offset",
        .defaultValue = 0.9f,
        .min          = 0.0f,
        .max          = 1.0f
    });
    chromatic_aberration.params.push_back({
        .displayName  = "Intensity",
        .defaultValue = 0.2f,
        .min          = 0.0f,
        .max          = 1.0f
    });

    EffectCreateInfo rotate_zoom("Affine Transformation", "build/src/shaders/affine_transformation.spv", {.usePushConstant = true, .pushConstantSize = 20});
    rotate_zoom.params.push_back({
        .displayName  = "Rotation",
        .defaultValue = 0.0f,
        .min          = -std::numbers::pi,
        .max          = std::numbers::pi
    });
    rotate_zoom.params.push_back({
        .displayName  = "Zoom",
        .defaultValue = 1.0f,
        .min          = 1e-6f,
        .max          = 2.0f
    });
    rotate_zoom.params.push_back({
        .displayName  = "PanX",
        .defaultValue = 0.0f,
        .min          = -2000.0f,
        .max          = 2000.0f
    });
    rotate_zoom.params.push_back({
        .displayName  = "PanY",
        .defaultValue = 0.0f,
        .min          = -2000.0f,
        .max          = 2000.0f
    });

    EffectCreateInfo gaussian_blur("Gaussian blur", "build/src/shaders/gaussian_blur.spv", {.usePushConstant = true, .pushConstantSize = 8}, 2);
    gaussian_blur.params.push_back({
        .displayName  = "Sigma",
        .defaultValue = 1.5f,
        .min          = 0.5f,
        .max          = 10.0f,
    });

    EffectCreateInfo canny("Canny", "build/src/shaders/canny.spv", {.usePushConstant = true, .pushConstantSize = 16}, 11);
    canny.params.push_back({
        .displayName  = "Sigma",
        .defaultValue = 1.5f,
        .min          = 0.5f,
        .max          = 10.0f,
    });
    canny.params.push_back({
        .displayName  = "Low threshold",
        .defaultValue = 0.05f,
        .min          = 0.0f,
        .max          = 0.2f,
    });
    canny.params.push_back({
        .displayName  = "High threshold",
        .defaultValue = 0.15f,
        .min          = 0.0f,
        .max          = 1.0f,
    });

    EffectCreateInfo mask("Background Remover", "build/src/shaders/mask.spv", {.usePushConstant = false, .pushConstantSize = 1});

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
    mEffectCreateInfos.push_back(pseudo_median_filter);
    mEffectCreateInfos.push_back(median_filter);
    mEffectCreateInfos.push_back(chromatic_aberration);
    mEffectCreateInfos.push_back(rotate_zoom);
    mEffectCreateInfos.push_back(gaussian_blur);
    mEffectCreateInfos.push_back(canny);
    mEffectCreateInfos.push_back(mask);
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
