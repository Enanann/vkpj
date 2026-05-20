#pragma once

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
    void initInputManager();
    void mainLoop();
    void cleanup();

    Window mWindow;
    std::unique_ptr<Renderer> mRenderer;
    std::unique_ptr<InputManager> mInputRenderer;
    std::unique_ptr<BackgroundRemover> mBackgroundRemover;
};
