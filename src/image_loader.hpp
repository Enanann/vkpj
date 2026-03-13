#pragma once

#include <stb/stb_image.h>

#include <filesystem>

struct ImageLoadResult {
    int texWidth{};
    int texHeight{};
    int texChannels{};
    stbi_uc* pixels;
};

class ImageLoader {
public:
    ImageLoader(std::filesystem::path, ImageLoadResult);

    const ImageLoadResult& getResult() const;

    static ImageLoader loadImageFromPath(const std::filesystem::path& path);
    
private:
    std::filesystem::path mPath;
    ImageLoadResult       mImageLoadResult;
};
