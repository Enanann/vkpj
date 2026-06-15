# vkpj (Vulkan Image processing)

An image processing application built with the Vulkan API. 

## Features

* Load image and navigate the view.
* Apply effects through Vulkan compute shader written in Slang.
* Apply background removal using IS-Net model via ncnn.
* Save processed image or save the background removal mask.
* Batch processing using selected effects with input and output folders.

## Dependencies

* **CMake** (v4.0 or higher)
* **Vulkan SDK**: [LunarG](https://vulkan.lunarg.com/sdk/home) (Slang shader compiler included, make sure to check the "Shader Toolchain Debug Symbols")
* **GLFW**: Window manager (Install with `vcpkg` or OS-specific package manager)
* **ncnn**: For background remover (Install with `vcpkg` or OS-specific package manager)

`stb_image`, `Dear ImGui` are in the `external` directory. No additional installation required.

## Build Instructions

If using vcpkg to install
```bash
vcpkg install glfw3 glm ncnn[vulkan]
```

After cloning the project (see below), download the models from the Release pages (The application uses the IS-Net model for best-quality background removal). Put the .param and .bin file into models/ (same directory as src/).

Build
```bash
git clone --recurse-submodules https://github.com/Enanann/vkpj.git
cd vkpj

cmake -B build -S . # Use -DCMAKE_TOOLCHAIN_FILE=[path/to/vcpkg]/scripts/buildsystems/vcpkg.cmake if using vcpkg
cmake --build build

./build/src/pkvk
```

Executable location may vary depending on generator (Debug/Release folders for Visual Studio builds).