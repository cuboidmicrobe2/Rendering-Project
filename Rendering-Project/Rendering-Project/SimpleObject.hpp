#pragma once

#include "ConstantBuffer.hpp"
#include "SceneObject.hpp"

class SimpleObject : public SceneObject {
  public:
    SimpleObject(Transform transform, Mesh* mesh, bool tesselate = true, bool showTessellation = false);
    /*SimpleObject(SimpleObject&) = default;*/
    ~SimpleObject() override {}

    void Draw(ID3D11Device* device, ID3D11DeviceContext* context) override;
    void Init(ID3D11Device* device) override;

  private:
};