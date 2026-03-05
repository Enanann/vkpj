#include "platform.hpp"
#include <GLFW/glfw3.h>

#include <iostream>

Window::Window() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
}

Window::~Window() {
    glfwDestroyWindow(mWindow);

    glfwTerminate();
}

void Window::create() {
    mWindow = glfwCreateWindow(MIN_WIDTH, MIN_HEIGHT, "pkvk", nullptr, nullptr);
    if (!mWindow) {
        std::cerr << "Can not create window";
        return;
    }

    glfwSetWindowUserPointer(mWindow, this);
    glfwSetFramebufferSizeCallback(mWindow, framebufferResizeCallback);
}

void Window::update() {
    glfwPollEvents();
}

void Window::wait() {
    glfwWaitEvents();
}

void Window::getFrameBufferSize(int* width, int* height) const {
    glfwGetFramebufferSize(mWindow, width, height);
}

bool Window::getResizeStatus() const {
    return mFramebufferResized;
}

void Window::setResizeStatus(bool value) {
    mFramebufferResized = value;
}
    
GLFWwindow* Window::getGLFWHandle() const {
    return mWindow;
}
