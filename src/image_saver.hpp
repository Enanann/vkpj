#pragma once

#include <filesystem>

class Renderer;

namespace ImageSaver {
    void initialize();
    void saveImage(std::filesystem::path& path, Renderer* renderer);
};
