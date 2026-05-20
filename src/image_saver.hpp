#pragma once

#include "rmbg.hpp"

#include <filesystem>
#include <atomic> // thread safe variable
#include <memory>

class Renderer;

struct SaveJob {
    std::atomic<bool> finished{false};
};

namespace ImageSaver {
    void initialize();
    std::shared_ptr<SaveJob> saveImage(std::filesystem::path& path, Renderer* renderer);
    std::shared_ptr<SaveJob> saveMask(std::filesystem::path& path, Renderer* renderer, BackgroundRemover* bgRemover);
};
