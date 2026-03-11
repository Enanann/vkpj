set(SOURCES 
    main.cpp
    ${IMGUI_SOURCES}
    app.cpp 
    platform.cpp
    read_binary.cpp 
    image_loader.cpp
    vertex.cpp
    imgui_system.cpp
    input_manager.cpp
    
    core/device.cpp 
    core/glfw_surface.cpp 
    core/instance.cpp 
    core/shader.cpp 
    core/swapchain.cpp 
    core/commandbuffer.cpp
    core/commandpool.cpp
    core/buffer.cpp
    core/descriptor_layout.cpp
    core/descriptor_pool.cpp
    core/descriptor_set.cpp
    
    image/image.cpp
    image/sampler.cpp

    pipeline/graphics_pipeline.cpp
    
    renderer/renderer.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../external
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/pipeline
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/image
    ${IMGUI_DIR}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE 
    Vulkan::cppm 
    glfw 
    
)

# C++ standard
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# Compile shaders automatically
add_slang_shader_target(
    ${PROJECT_NAME}_shaders
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader_base.slang"
)
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_shaders)
