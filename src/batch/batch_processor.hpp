#pragma once

#include <filesystem>
#include <future>
#include <vector>

class Renderer;
enum class SaveAction;

class BatchProcessor {
public:
    BatchProcessor(Renderer*);

    void start(const std::filesystem::path& inputDir, const std::filesystem::path& outputDir, SaveAction action);
    void update();

    bool isProcessing() const;
    float getProcess() const;
    size_t getCurrentIndex() const;
    size_t getTotalCount() const;

private:
    Renderer* mRenderer;

    bool                               mIsProcessing{false};
    std::vector<std::filesystem::path> mFiles;
    size_t                             mCurrentIndex{0};
    std::filesystem::path              mOutputDir;
    SaveAction                         mAction;

    std::vector<std::future<bool>>     mFutures;

    const size_t MAX_IN_FLIGHT = 8;
};
