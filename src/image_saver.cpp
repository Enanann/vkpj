#include "image_saver.hpp"
#include "renderer.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <vulkan/vulkan.hpp>

#include <iostream>
#include <thread>
#include <vector>

ImageSaver::ImageSaver() : mPool{std::thread::hardware_concurrency() - 1} {
}

void ImageSaver::initialize() {
    stbi_flip_vertically_on_write(true);
}

std::future<bool> ImageSaver::saveImage(const std::filesystem::path& path, std::vector<unsigned char> imageData, int width, int height, vk::DeviceSize rowPitch) {
    return mPool.enqueue([p = std::move(path), data = std::move(imageData), width, height, rowPitch] () -> bool {
        // filesystem::path::c_str() returns path::value_type*, which is OS-dependent (const wchar_t* on Windows vs const char* on Linux)
        int success{stbi_write_png(p.string().c_str(), width, height, 4, data.data(), rowPitch)};
    
        if (success) {
            std::cout << "Image successfully saved to disk at " << p << std::endl;
            return true;
        } else {
            std::cout << "Image not successfully saved to disk" << std::endl;
            return false;
        }
    });
}
