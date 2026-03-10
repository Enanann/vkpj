#pragma once

#include "platform.hpp"
#include "renderer.hpp"
class InputManager {
public:
    InputManager(Window&, Renderer&);

    void processInput();
private:
    Window&   mWindow;
    Renderer& mRenderer;
    float     mPanSpeed{1.36f};
    glm::vec2 mPan{0.0f, 0.0f};
    glm::vec2 mLastMousePos;
    float     mZoom{45.0f};
};
