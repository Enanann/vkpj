#include "app.hpp"
#include "batch/batch_processor.hpp"
#include "imgui_system.hpp"
#include "input_manager.hpp"
#include "renderer.hpp"
#include "rmbg.hpp"
#include <gpu.h>
#include <memory>

void Application::run() {
    ncnn::create_gpu_instance();
    Application::initNcnn();
    Application::initWindow();
    Application::initRenderer();
    Application::initBatchProcessor();
    Application::initImGuiSystem();
    Application::initInputManager();
    Application::mainLoop();
    Application::cleanup();
}

void Application::initNcnn() {
    mBackgroundRemover = std::make_unique<BackgroundRemover>("models/model_isnet.ncnn.param", "models/model_isnet.ncnn.bin"); // isnet
    // mBackgroundRemover = std::make_unique<BackgroundRemover>("models/model_modnet.ncnn.param", "models/model_modnet.ncnn.bin"); // modnet
    // mBackgroundRemover = std::make_unique<BackgroundRemover>("models/model_rmbg14.ncnn.param", "models/model_rmbg14.ncnn.bin"); // rmbg-1.4
}

void Application::initWindow() {
    mWindow.create();
}

void Application::initRenderer() {
    mRenderer = std::make_unique<Renderer>(mWindow, *mBackgroundRemover.get());
}

void Application::initBatchProcessor() {
    mBatchProcessor = std::make_unique<BatchProcessor>(mRenderer.get());
}

void Application::initImGuiSystem() {
    mImGuiSystem = std::make_unique<ImGuiSystem>(mRenderer.get(), mBackgroundRemover.get(), mBatchProcessor.get());
}

void Application::initInputManager() {
    mInputRenderer = std::make_unique<InputManager>(mWindow, *mRenderer);
}

void Application::mainLoop() {
    while (!glfwWindowShouldClose(mWindow.getGLFWHandle())) {
        mWindow.update();
        mInputRenderer->processInput();
        // mRenderer->drawImGui();
        mImGuiSystem->newFrame();
        mImGuiSystem->render();
        mRenderer->draw();
        mBatchProcessor->update();
    }
    mRenderer->getDevice().getVkHandle().waitIdle();
    mRenderer->cleanup();
}

void Application::cleanup() {
    mImGuiSystem->cleanup();
    // glfwTerminate();
    // ncnn::destroy_gpu_instance();
}
