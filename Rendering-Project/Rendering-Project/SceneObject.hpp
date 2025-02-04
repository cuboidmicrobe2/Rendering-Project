#ifndef SCENE_OBJECT_HPP
#define SCENE_OBJECT_HPP

#include <d3d11.h>
#include <array>
#include <vector>
#include "SimpleVertex.hpp"

class SceneObject {
    std::array<float, 3> position;
    std::vector<SimpleVertex> mesh;


    SceneObject(std::array<float, 3> position, std::vector<SimpleVertex> mesh);

};

#endif