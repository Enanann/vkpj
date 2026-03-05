#pragma once

#include "platform.hpp"
#include "renderer.hpp"
#include <memory>

class Application {
public:
    void run();
private:
    void initWindow(); 
    void initRenderer();
    void mainLoop();
    void cleanup();

    Window mWindow;
    std::unique_ptr<Renderer> mRenderer;
};
