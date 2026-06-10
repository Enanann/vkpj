#pragma once

#include "batch/batch_processor.hpp"
#include "imgui_system.hpp"
#include "platform.hpp"
#include "renderer.hpp"
#include "input_manager.hpp"
#include "rmbg.hpp"
#include <memory>

class Application {
public:
    void run();
private:
    void initNcnn();
    void initWindow(); 
    void initRenderer();
    void initBatchProcessor();
    void initImGuiSystem();
    void initInputManager();
    void mainLoop();
    void cleanup();

    Window mWindow;
    std::unique_ptr<Renderer> mRenderer;
    std::unique_ptr<InputManager> mInputRenderer;
    std::unique_ptr<BackgroundRemover> mBackgroundRemover;
    std::unique_ptr<BatchProcessor> mBatchProcessor;
    std::unique_ptr<ImGuiSystem> mImGuiSystem;
};
