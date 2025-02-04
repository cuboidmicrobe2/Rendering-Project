#include "SceneObject.hpp"

SceneObject::SceneObject(const DirectX::XMVECTOR& position, Mesh& mesh)
    : position(position), mesh(&mesh) {}

