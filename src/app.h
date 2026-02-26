#pragma once

#include "platform.h"

#include <memory>

class Application {
public:
    void run();
private:
    void initWindow(); 
    void initVulkan();
    void mainLoop();
    void cleanup();

    Window mWindow;
};