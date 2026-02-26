#include "platform.h"

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
    mWindow = glfwCreateWindow(MIN_WIDTH, MIN_HEIGHT, "ILovePoki", nullptr, nullptr);
}

GLFWwindow* Window::getWindow() {
    return mWindow;
}