#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

static const uint32_t MIN_WIDTH  = 800;
static const uint32_t MIN_HEIGHT = 600;

class Window {
public:
    Window();
    ~Window();

    void create();

    GLFWwindow* getWindow();
private:
    GLFWwindow* mWindow{nullptr};
};