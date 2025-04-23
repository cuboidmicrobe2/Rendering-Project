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
    virtual ~SceneObject() {};

    virtual void Draw(ID3D11Device* device, ID3D11DeviceContext* context) = 0;
    void DrawMesh(ID3D11DeviceContext* context);
    SceneObject(SceneObject&)                                                   = delete;

    DirectX::XMFLOAT4X4 GetWorldMatrix() const;
    virtual void Init(ID3D11Device* device) = 0;

    virtual void Update();

    void SetBoundingBox(DirectX::BoundingBox& boundingBox);
    DirectX::BoundingBox GetBoundingBox() const;

  protected:
    DirectX::BoundingBox boundingBox;
    Mesh* mesh;
};

#endif