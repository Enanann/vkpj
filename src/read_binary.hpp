#pragma once

#include <filesystem>
#include <vector>

namespace BinaryReader {
    std::vector<char> readFile(const std::filesystem::path& filepath);
    std::vector<char> readFile(const std::string& filepath);
};
