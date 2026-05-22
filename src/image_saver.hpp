#pragma once

#include "threadpool.hpp"

#include <vulkan/vulkan.hpp>

#include <filesystem>

class Renderer;

class ImageSaver {
public:
    ImageSaver();

    void initialize();

    std::future<bool> saveImage(const std::filesystem::path& path, std::vector<unsigned char> imageData, int width, int height, vk::DeviceSize rowPitch);

private:
    ThreadPool mPool;
};
