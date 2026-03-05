#include "read_binary.hpp"

#include <fstream>
#include <stdexcept>
// #include <ios>

std::vector<char> BinaryReader::readFile(const std::filesystem::path &filepath) {
    std::ifstream file(filepath.string(), std::ios::ate | std::ios::binary);

    std::vector<char> buffer(file.tellg());

    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    file.close();

    return buffer;
}

std::vector<char> BinaryReader::readFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);
    if (!file) {
        throw std::runtime_error("Can't open file with path: " + filepath + '\n');
    }

    std::vector<char> buffer(file.tellg());

    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    file.close();

    return buffer;
}
