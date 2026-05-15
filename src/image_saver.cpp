#include "image_saver.hpp"

#include "image_loader.hpp"
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

void ImageSaver::saveImage(std::filesystem::path& path, Renderer* renderer) {
    int width;
    int height;
    vk::DeviceSize rowPitch;
    auto imageData = renderer->getCurrentImageData(width, height, rowPitch);

    /*
    If we can't use blit (which does automatic conversion), we don't need to manually swizzle color components 
    since both the source and destination use the same RGB format
    */
   std::thread worker([path = std::move(path), width, height, data = std::move(imageData), rowPitch]() {
       // filesystem::path::c_str() returns path::value_type*, which is OS-dependent (const wchar_t* on Windows vs const char* on Linux)
       int success{stbi_write_png(path.string().c_str(), width, height, 4, data.data(), rowPitch)};
    
       if (success) {
           std::cout << "Image successfully saved to disk at " << path << std::endl;
       } else {
           std::cout << "Image not successfully saved to disk" << std::endl;
       }
   });
   worker.detach();
}
