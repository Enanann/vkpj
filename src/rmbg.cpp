#include "rmbg.hpp"
#include "stb/stb_image_write.h"
#include <ncnn/mat.h>
#include <iostream>

BackgroundRemover::BackgroundRemover(const std::filesystem::path& param, const std::filesystem::path& bin) {
    mNet.opt.use_vulkan_compute = true;
    mNet.set_vulkan_device(ncnn::get_gpu_device(0));

    mNet.opt.use_fp16_packed = false;
    mNet.opt.use_fp16_storage = false;
    mNet.opt.use_fp16_arithmetic = false;

    if (mNet.load_param(param.string().c_str()) != 0) {
        std::cerr << "Failed to load param file!" << std::endl;
    }
    
    if (mNet.load_model(bin.string().c_str()) != 0) {
        std::cerr << "Failed to load bin file!" << std::endl;
    }
}

std::vector<float> BackgroundRemover::generateMask(unsigned char* raw_data, int width, int height) {
    // Resize and convert
    ncnn::Mat in = ncnn::Mat::from_pixels_resize(
        raw_data, 
        ncnn::Mat::PixelType::PIXEL_RGBA2RGB, 
        width, height,
        1024, 1024 
    );

    const float mean_vals[3] = {127.5f, 127.5f, 127.5f};
    const float norm_vals[3] = {1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f}; 
    in.substract_mean_normalize(mean_vals, norm_vals);

    // Inference
    ncnn::Extractor ex = mNet.create_extractor();
    ex.input("in0", in); 
    
    ncnn::Mat out;
    ex.extract("out0", out);

    // Resize back to original image dimensions
    ncnn::Mat finalMask;
    ncnn::resize_bilinear(out, finalMask, width, height);

    float* raw_mask_floats = static_cast<float*>(finalMask.data);
    int total_pixels = width * height;

    std::vector<float> mask(total_pixels);
    std::vector<unsigned char> debug_vision(total_pixels);

    for (int i = 0; i < total_pixels; ++i) {
        float alpha = std::clamp(raw_mask_floats[i], 0.0f, 1.0f);
        mask[i] = alpha;

        debug_vision[i] = static_cast<unsigned char>(alpha * 255.0f);
    }

#ifdef DEBUG_MASK
    stbi_write_png("TRUE_MODNET_MASK.png", width, height, 1, debug_vision.data(), width);
#endif

    return mask;
}
