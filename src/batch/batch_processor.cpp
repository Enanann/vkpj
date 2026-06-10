#include "batch_processor.hpp"

#include "imgui_system.hpp"
#include "renderer.hpp"

#include <algorithm>
#include <chrono>

BatchProcessor::BatchProcessor(Renderer* renderer) : mRenderer{renderer} {}

void BatchProcessor::start(const std::filesystem::path& inputDir, const std::filesystem::path& outputDir, SaveAction action) {
    mFiles.clear();

    for (const auto& entry : std::filesystem::directory_iterator(inputDir)) {
        if (entry.is_regular_file()) {
            auto extension{entry.path().extension()};
            if (extension == ".png" || extension == ".jpg" || extension == ".jpeg") {
                mFiles.push_back(entry);
            }
        }

    }
    if (!mFiles.empty()) {
        mOutputDir = outputDir;
        mAction = action;
        mCurrentIndex = 0;
        mIsProcessing = true;
    }
}

void BatchProcessor::update() {
    if (!mIsProcessing) return;

    mFutures.erase(
        std::remove_if(mFutures.begin(), mFutures.end(), [](const std::future<bool>& f) {
            return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
        }),
        mFutures.end()
    );

    if (mFutures.size() >= MAX_IN_FLIGHT) return;

    if (mCurrentIndex < mFiles.size()) {
        const auto& inputPath = mFiles[mCurrentIndex];
        std::filesystem::path outputPath = mOutputDir / inputPath.filename();
        outputPath.replace_extension(".png");

        bool process{true};
        if (!mRenderer->changeImage(inputPath)) process = false;
        mRenderer->runCompute();

        if (!process) {

        } else if (mAction == SaveAction::SaveImage) {
            mFutures.push_back(mRenderer->saveCurrentImage(outputPath));
        } else if (mAction == SaveAction::SaveMask) {
            mFutures.push_back(mRenderer->saveCurrentImageMask(outputPath));
        }
        mCurrentIndex++;
    } else if (mFutures.empty()) {
        mIsProcessing = false;
        std::cout << "Finished processing folder" << std::endl;
    }
}

bool BatchProcessor::isProcessing() const {
    return mIsProcessing;
}

float BatchProcessor::getProcess() const {
    if (mFiles.empty()) return 0.0f;
    return static_cast<float>(mCurrentIndex) / mFiles.size() * 100;
}

size_t BatchProcessor::getCurrentIndex() const {
    return mCurrentIndex;
}

size_t BatchProcessor::getTotalCount() const {
    return mFiles.size();
}
