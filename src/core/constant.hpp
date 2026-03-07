#pragma once

#include "vertex.hpp"
#include <vector>

inline constexpr int MAX_FRAMES_IN_FLIGHT = 2;

inline const std::vector<Vertex> gVertices = {{
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
}};

inline const std::vector<uint16_t> gIndices = {
    0, 1, 2, 2, 3, 0
};
