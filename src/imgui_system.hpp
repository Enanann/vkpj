#pragma once

#include "batch/batch_processor.hpp"
#include "descriptor_pool.hpp"
#include "imgui.h"
#include "imfilebrowser.h"
#include "image_saver.hpp"
#include <array>
#include <filesystem>

class Renderer;
class DescriptorPool;
class BackgroundRemover;

enum class SaveAction {
    None,
    SaveImage,
    SaveMask
};

enum class FolderState {
    None,
    Input,
    Output
}; 

class ImGuiSystem {
public:
    ImGuiSystem(Renderer*, BackgroundRemover*, BatchProcessor*);

    void newFrame();
    void render();

    void setupDocking();

    void cleanup();
private:
    Renderer*      mRenderer;
    BackgroundRemover* mBackgroundRemover;
    BatchProcessor* mBatchProcessor;
    DescriptorPool mDescriptorPool;

    ImGui::FileBrowser mFilebrowser;
    ImGui::FileBrowser mDirectoryBrowser{ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CreateNewDir};   
    ImGui::FileBrowser mFolderBrowser{ImGuiFileBrowserFlags_SelectDirectory};
    std::array<std::filesystem::path, 2> mFolders;
    FolderState mFolderState;

    // std::shared_ptr<SaveJob> mSaveJob;
    std::future<bool> mFinished;
    SaveAction        mSaveAction;
    
    bool mShowDemoWindow{true};
};
