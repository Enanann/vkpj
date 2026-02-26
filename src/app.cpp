#include "app.h"

void Application::run() {
    Application::initWindow();
    Application::initVulkan();
    Application::mainLoop();
    Application::cleanup();
}

void Application::initWindow() {
    mWindow.create();
}

void Application::mainLoop() {
    while (!glfwWindowShouldClose(mWindow.getWindow())) {
        glfwPollEvents();
    }
}

void Application::initVulkan() {

}

void Application::cleanup() {
    // glfwTerminate();
}