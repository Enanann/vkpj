#pragma once

#include <stb/stb_image.h>

#include <filesystem>
#include <vector>

struct ImageLoadResult {
    int texWidth{};
    int texHeight{};
    int texChannels{};
    std::vector<stbi_uc> pixels;
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
