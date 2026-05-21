#include "image_loader.hpp"
#include <stdexcept>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

ImageLoader::ImageLoader(std::filesystem::path path, ImageLoadResult result, bool valid) : mPath{path}, mImageLoadResult{result} {
    mValid = valid;
}

const ImageLoadResult& ImageLoader::getResult() const {
    return mImageLoadResult;
}

ImageLoader::ImageLoader() {
    
}

ImageLoader ImageLoader::loadImageFromPath(const std::filesystem::path& path) {
    stbi_set_flip_vertically_on_load(true);
    ImageLoadResult res;
    bool valid{true};
    // filesystem::path::c_str() returns path::value_type*, which is OS-dependent (const wchar_t* on Windows vs const char* on Linux)
    auto raw = stbi_load(path.string().c_str(), &res.texWidth, &res.texHeight, &res.texChannels, STBI_rgb_alpha);
    size_t size = res.texWidth * res.texHeight * 4;
    res.pixels.assign(raw, raw + size);

    if (!raw) {
        // throw std::runtime_error("Failed to load image from path: " + path.string() + '\n');
        std::cerr << "Failed to load image from path: " << path.string() << std::endl;
        valid = false;
    }

    stbi_image_free(raw);

    return ImageLoader(path, res, valid);
}
