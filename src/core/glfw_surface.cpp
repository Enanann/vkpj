#include "glfw_surface.hpp"

#include "instance.hpp"
#include "platform.hpp"

#include <stdexcept>

GLFWSurface::GLFWSurface(Instance& instance, Window& window) 
    : mInstance{instance}
    , mWindow{window} 
{
    VkSurfaceKHR _surface;
    if (glfwCreateWindowSurface(*mInstance.getVkHandle(), mWindow.getGLFWHandle(), nullptr, &_surface) != 0) {
        throw std::runtime_error("Failed to create window surface");
    }

    mSurface = vk::raii::SurfaceKHR(mInstance.getVkHandle(), _surface);
}

const vk::SurfaceKHR GLFWSurface::getVkHandle() const {
    return *mSurface;
}
