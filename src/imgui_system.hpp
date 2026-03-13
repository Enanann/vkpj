#pragma once

#include "descriptor_pool.hpp"
#include "imgui.h"
#include "imfilebrowser.h"

class Renderer;
class DescriptorPool;

class ImGuiSystem {
public:
    ImGuiSystem(Renderer*);

    void newFrame();
    void render();

    void setupDocking();

    void cleanup();
private:
    Renderer*      mRenderer;
    DescriptorPool mDescriptorPool;

    ImGui::FileBrowser mFilebrowser;
    
    bool mShowDemoWindow{true};
};
