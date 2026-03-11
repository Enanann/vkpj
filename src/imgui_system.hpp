#pragma once

#include "descriptor_pool.hpp"

class Renderer;
class DescriptorPool;

class ImGuiSystem {
public:
    ImGuiSystem(const Renderer*);

    void newFrame();
    void render();

    void setupDocking();
private:
    const Renderer* mRenderer;

    DescriptorPool mDescriptorPool;

    bool mShowDemoWindow{true};
};
