#pragma once

#include "ConstantBuffer.hpp"
#include "SceneObject.hpp"

class SimpleObject : public SceneObject {
  public:
    SimpleObject(Transform transform, Mesh* mesh);
    SimpleObject(SimpleObject&) = default;
    void Draw(ID3D11Device* device, ID3D11DeviceContext* context) const override;

  private:
    Mesh* mesh;
};