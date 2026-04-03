#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <utility>

inline const uint32_t MIN_WIDTH  = 1280;
inline const uint32_t MIN_HEIGHT = 720;
inline const float    ZOOM_SPEED = 0.7f;

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
    std::pair<double, double> getMousePos();
    double getScrollOffset();

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto _window{reinterpret_cast<Window*>(glfwGetWindowUserPointer(window))};
        _window->setResizeStatus(true);
    }

    static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
        auto _window{reinterpret_cast<Window*>(glfwGetWindowUserPointer(window))};
        _window->mMouseX = xpos;
        _window->mMouseY = ypos;
    }

    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        auto _window{reinterpret_cast<Window*>(glfwGetWindowUserPointer(window))};
        if (glfwGetKey(_window->getGLFWHandle(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            _window->mScrollOffset -= yoffset * ZOOM_SPEED;
        }
    }

    GLFWwindow* getGLFWHandle() const;
private:
    GLFWwindow* mWindow{nullptr};
    double mMouseX;
    double mMouseY;
    double mScrollOffset{45.0f};
    bool mFramebufferResized{false};
};
