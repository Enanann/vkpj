#pragma once

#include "descriptor_pool.hpp"
#include "imgui.h"
#include "imfilebrowser.h"
#include "image_saver.hpp"

class Renderer;
class DescriptorPool;
class BackgroundRemover;

enum class SaveAction {
    None,
    SaveImage,
    SaveMask
};

class ImGuiSystem {
public:
    ImGuiSystem(Renderer*, BackgroundRemover*);

    void newFrame();
    void render();

    void setupDocking();

    void cleanup();
private:
    Renderer*      mRenderer;
    BackgroundRemover* mBackgroundRemover;
    DescriptorPool mDescriptorPool;

    ImGui::FileBrowser mFilebrowser;
    ImGui::FileBrowser mDirectoryBrowser{ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CreateNewDir};   
    
    std::shared_ptr<SaveJob> mSaveJob;
    SaveAction               mSaveAction;
    
    bool mShowDemoWindow{true};
};
