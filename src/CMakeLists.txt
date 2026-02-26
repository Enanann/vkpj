set(SOURCES 
    main.cpp
    ${IMGUI_SOURCES}
    app.cpp 
    platform.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../external
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
