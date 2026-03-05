#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

inline const uint32_t MIN_WIDTH  = 800;
inline const uint32_t MIN_HEIGHT = 600;

class Window {
public:
    Window();
    ~Window();

    void create();
    void update();
    void wait();
    void getFrameBufferSize(int* width, int* height) const;
    bool getResizeStatus() const;
    void setResizeStatus(bool);

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto _window{reinterpret_cast<Window*>(glfwGetWindowUserPointer(window))};
        _window->setResizeStatus(true);
    }

    GLFWwindow* getGLFWHandle() const;
private:
    GLFWwindow* mWindow{nullptr};
    bool mFramebufferResized{false};
};
