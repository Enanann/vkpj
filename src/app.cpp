#include "app.hpp"
#include "renderer.hpp"
#include <memory>

void Application::run() {
    Application::initWindow();
    Application::initRenderer();
    Application::mainLoop();
    Application::cleanup();
}

void Application::initWindow() {
    mWindow.create();
}

void Application::mainLoop() {
    while (!glfwWindowShouldClose(mWindow.getGLFWHandle())) {
        mWindow.update();
        mRenderer->draw();
    }
    mRenderer->getDevice().getVkHandle().waitIdle();
}

void Application::initRenderer() {
    mRenderer = std::make_unique<Renderer>(mWindow);
}

void Application::cleanup() {
    // glfwTerminate();
}
