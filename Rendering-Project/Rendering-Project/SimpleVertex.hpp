#ifndef SIMPLE_VERTEX_HPP
#define SIMPLE_VERTEX_HPP

#include <array>

struct SimpleVertex {
    std::array<float, 3> position;
    std::array<float, 3> normal;
    std::array<float, 2> uv;
};

#endif