#include "image_saver.hpp"
#include "renderer.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <vulkan/vulkan.hpp>

#include <iostream>
#include <thread>
#include <vector>

void ImageSaver::initialize() {
    stbi_flip_vertically_on_write(true);
}

std::shared_ptr<SaveJob> ImageSaver::saveImage(std::filesystem::path& path, Renderer* renderer) {
    int width;
    int height;
    vk::DeviceSize rowPitch;
    auto imageData = renderer->getCurrentImageData(width, height, rowPitch);

    /*
    If we can't use blit (which does automatic conversion), we don't need to manually swizzle color components 
    since both the source and destination use the same RGB format
    */
   auto isFinished = std::make_shared<SaveJob>();
   std::thread worker([isFinished, path = std::move(path), width, height, data = std::move(imageData), rowPitch]() {
       // filesystem::path::c_str() returns path::value_type*, which is OS-dependent (const wchar_t* on Windows vs const char* on Linux)
       int success{stbi_write_png(path.string().c_str(), width, height, 4, data.data(), rowPitch)};
    
       if (success) {
           std::cout << "Image successfully saved to disk at " << path << std::endl;
       } else {
           std::cout << "Image not successfully saved to disk" << std::endl;
       }
       isFinished->finished = true;
   });
   worker.detach();

   return isFinished;
}

std::shared_ptr<SaveJob> ImageSaver::saveMask(std::filesystem::path& path, Renderer* renderer, BackgroundRemover* bgRemover) {
    int width;
    int height;
    vk::DeviceSize rowPitch;
    
    auto imageData = renderer->getCurrentImageData(width, height, rowPitch);
    
    // Pack the Vulkan buffer for the model
    std::vector<unsigned char> tightlyPackedRgba(width * height * 4);
    for (int y = 0; y < height; ++y) {
        memcpy(
            tightlyPackedRgba.data() + (y * width * 4), 
            imageData.data() + (y * rowPitch), 
            width * 4
        );
    }

    auto mask = bgRemover->generateMask(tightlyPackedRgba.data(), width, height);

    // Calculate the alpha
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int maskIndex = (y * width) + x;
            int pixelOffset = (y * rowPitch) + (x * 4);

            float alpha_float = mask[maskIndex]; 
            unsigned char alpha_byte = static_cast<unsigned char>(alpha_float * 255.0f);

            imageData[pixelOffset + 0] = alpha_byte; // R
            imageData[pixelOffset + 1] = alpha_byte; // G
            imageData[pixelOffset + 2] = alpha_byte; // B
            imageData[pixelOffset + 3] = alpha_byte; // A
        }
    }

    auto isFinished = std::make_shared<SaveJob>();
    isFinished->finished = false;
   
    std::thread worker([isFinished, p = std::move(path), width, height, data = std::move(imageData), rowPitch]() {
        int success = stbi_write_png(p.string().c_str(), width, height, 4, data.data(), rowPitch);
    
        if (success) {
            std::cout << "Mask successfully saved to disk at " << p << std::endl;
        } else {
            std::cout << "Mask not successfully saved to disk" << std::endl;
        }
        isFinished->finished = true;
    });
   
    worker.detach();

    return isFinished;
}
