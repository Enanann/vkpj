#pragma once

#include <vulkan/vulkan_raii.hpp>

class Instance;
class Window;

class GLFWSurface {
public:
    GLFWSurface(Instance& instace, Window& window);

    const vk::SurfaceKHR getVkHandle() const;
private:
    Instance& mInstance;
    Window& mWindow;

    vk::raii::SurfaceKHR mSurface{nullptr};
};
