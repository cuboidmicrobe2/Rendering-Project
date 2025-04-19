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

    SceneObject(Transform transform /*, Mesh* mesh*/);
    ~SceneObject() {};

    virtual void Draw(ID3D11Device* device, ID3D11DeviceContext* context) const = 0;
    SceneObject(SceneObject&)                                                   = delete;

    DirectX::XMMATRIX GetWorldMatrixMatrix();
    void InitBuffer(ID3D11Device* device);

    virtual void Update();

    void SetBoundingBox(DirectX::BoundingBox& boundingBox);
    DirectX::BoundingBox GetBoundingBox() const;

  protected:
    DirectX::XMFLOAT4X4 GetWorldMatrix() const;
    ConstantBuffer matrixBuffer;
    DirectX::BoundingBox boundingBox;
};

#endif