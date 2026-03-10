#include "image_loader.hpp"
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

ImageLoader::ImageLoader(std::filesystem::path path, ImageLoadResult result) : mPath{path}, mImageLoadResult{result} {

}

ImageLoader::~ImageLoader() {
    stbi_image_free(mImageLoadResult.pixels);
}

const ImageLoadResult& ImageLoader::getResult() const {
    return mImageLoadResult;
}

ImageLoader ImageLoader::loadImageFromPath(const std::filesystem::path& path) {
    stbi_set_flip_vertically_on_load(true);
    ImageLoadResult res;
    res.pixels = stbi_load(path.c_str(), &res.texWidth, &res.texHeight, &res.texChannels, STBI_rgb_alpha);
    if (!res.pixels) {
        throw std::runtime_error("Failed to load image from path: " + path.string() + '\n');
    }
    return ImageLoader(path, res);
}
