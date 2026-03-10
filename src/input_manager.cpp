#include "input_manager.hpp"
#include "renderer.hpp"
#include <GLFW/glfw3.h>

InputManager::InputManager(Window& window, Renderer& renderer) : mWindow{window}, mRenderer{renderer} {

}

void InputManager::processInput() {
    auto [currentX, currentY] = mWindow.getMousePos();
    glm::vec2 currentPos{static_cast<float>(currentX), static_cast<float>(currentY)};
    int width{};
    int height{};
    glfwGetWindowSize(mWindow.getGLFWHandle(), &width, &height);

    if (glfwGetKey(mWindow.getGLFWHandle(), GLFW_KEY_LEFT_ALT) == GLFW_PRESS &&
        glfwGetMouseButton(mWindow.getGLFWHandle(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        
        glm::vec2 delta = glm::vec2(currentPos.x - mLastMousePos.x, mLastMousePos.y - currentPos.y) * mPanSpeed * (mZoom / 45.0f);
        delta.x *= 2.0f / width;
        delta.y *= 2.0f / height; 
        mPan += delta;
        mRenderer.setPan(mPan);
    }

    mLastMousePos = currentPos;

    mZoom = static_cast<float>(mWindow.getScrollOffset());
    if (mZoom < 1.0f) {
        mZoom = 1.0f;
    }
    if (mZoom > 45.0f) {
        mZoom = 45.0f;
    }
    mRenderer.setZoom(mZoom);
}
