#ifndef SCENE_OBJECT_HPP
#define SCENE_OBJECT_HPP

#include "ConstantBuffer.hpp"
#include "Mesh.hpp"
#include "SimpleVertex.hpp"
#include "Transform.hpp"
#include <DirectXMath.h>
#include <array>
#include <d3d11.h>
#include <vector>

class SceneObject {
  public:
    Transform transform;

    SceneObject(Transform transform, Mesh* mesh);
    SceneObject(SceneObject&) = delete;
    ~SceneObject() {};

    void InitBuffer(ID3D11Device* device);

    void Draw(ID3D11Device* device, ID3D11DeviceContext* context);

    void Update();

  private:
    DirectX::XMFLOAT4X4 GetWorldMatrix() const;
    Mesh* mesh;
    ConstantBuffer matrixBuffer;
};

#endif