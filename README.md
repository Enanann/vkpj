# vkpj (Vulkan Image Pre-processing)

An image pre-processing application built with the Vulkan API. Users can load images into a 2D space, navigate the view, and apply various image effects directly on the GPU using Compute Shaders.

## Dependencies

* **CMake** (v4.0 or higher)
* **Vulkan SDK:** [LunarG](https://vulkan.lunarg.com/sdk/home) (Slang shader compiler included)
* **GLFW:** Window manager (Install with `vcpkg` or OS-specific package manager)

`GLM`, `stb_image`, `Dear ImGui` are in the `external` directory. No additional installation required

## Build Instructions

If using vcpkg to install GLFW
```bash
vcpkg install glfw3
```

Build
```bash
git clone https://github.com/Enanann/vkpj.git
cd vkpj

cmake -B build -S . # Use -DCMAKE_TOOLCHAIN_FILE=[path/to/vcpkg]/scripts/buildsystems/vcpkg.cmake if using vcpkg
cmake --build build

./build/src/pkvk #  ./build/src/Debug/pkvk.exe on Windows 
```