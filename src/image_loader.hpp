#pragma once

#include <stb/stb_image.h>

#include <filesystem>

struct ImageLoadResult {
    int texWidth{};
    int texHeight{};
    int texChannels{};
    stbi_uc* pixels{nullptr};
};

class ImageLoader {
public:
    bool mValid;

    ImageLoader();
    ImageLoader(std::filesystem::path, ImageLoadResult, bool);

    const ImageLoadResult& getResult() const;

    static ImageLoader loadImageFromPath(const std::filesystem::path& path);
    
private:
    std::filesystem::path mPath;
    ImageLoadResult       mImageLoadResult;
};
