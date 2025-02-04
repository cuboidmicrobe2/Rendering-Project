#ifndef SCENE_OBJECT_HPP
#define SCENE_OBJECT_HPP

#include <d3d11.h>
#include <array>
#include <vector>
#include "SimpleVertex.hpp"
#include "Mesh.hpp"
#include <DirectXMath.h>

class SceneObject {
    DirectX::XMVECTOR position;
    Mesh* mesh;

    SceneObject(const DirectX::XMVECTOR& position, Mesh& mesh);

};

#endif