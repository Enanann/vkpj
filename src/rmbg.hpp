#pragma once

#include <vulkan/vulkan.h>

#define NCNN_SIMPLEVK_H // avoid conflict of simplevk.h (ncnn) with official vulkan 
#include <ncnn/net.h>
#include <ncnn/gpu.h>

#include <filesystem>
#include <vector>

class BackgroundRemover {
public:
    BackgroundRemover(const std::filesystem::path& param, const std::filesystem::path& bin);
    std::vector<float> generateMask(unsigned char* raw_data, int width, int height); 

private:
    ncnn::Net mNet;
};
