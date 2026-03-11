#include "app.hpp"
#include "input_manager.hpp"
#include "renderer.hpp"
#include <memory>

void Application::run() {
    Application::initWindow();
    Application::initRenderer();
    Application::initInputManager();
    Application::mainLoop();
    Application::cleanup();
}

void Application::initWindow() {
    mWindow.create();
}

void Application::initRenderer() {
    mRenderer = std::make_unique<Renderer>(mWindow);
}

void Application::initInputManager() {
    mInputRenderer = std::make_unique<InputManager>(mWindow, *mRenderer);
}

void Application::mainLoop() {
    while (!glfwWindowShouldClose(mWindow.getGLFWHandle())) {
        mWindow.update();
        mInputRenderer->processInput();
        mRenderer->drawImGui();
        mRenderer->draw();
    }
    mRenderer->getDevice().getVkHandle().waitIdle();
}

void Application::cleanup() {
    // glfwTerminate();
}
