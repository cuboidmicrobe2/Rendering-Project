#ifndef SCENE_OBJECT_HPP
#define SCENE_OBJECT_HPP

#include "Mesh.hpp"
#include "SimpleVertex.hpp"
#include <DirectXMath.h>
#include <array>
#include <d3d11.h>
#include <vector>

class SceneObject {
  public:
    SceneObject(const DirectX::XMVECTOR& position, Mesh& mesh);

  private:
    DirectX::XMVECTOR position;
    Mesh* mesh;
};

#endif