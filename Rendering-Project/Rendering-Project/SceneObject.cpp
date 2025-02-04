#include "SceneObject.hpp"

SceneObject::SceneObject(std::array<float, 3> position, std::vector<SimpleVertex> mesh)
    : position(position), mesh(mesh) {}