#include "SceneObject.hpp"
#include "ConstantBuffer.hpp"

class SimpleObject : public SceneObject {
  public:
    SimpleObject(Transform transform, Mesh* mesh);
    void Draw(ID3D11Device* device, ID3D11DeviceContext* context) const override;

  private:
    Mesh* mesh;
};

SimpleObject::SimpleObject(Transform transform, Mesh* mesh) : SceneObject(transform), mesh(mesh) {}

void SimpleObject::Draw(ID3D11Device* device, ID3D11DeviceContext* context) const {
    // Write Object Worldmatrix to vertexShader
    ConstantBuffer buffer;
    DirectX::XMFLOAT4X4 worldMatrix = this->GetWorldMatrix();
    buffer.Initialize(device, sizeof(worldMatrix), &worldMatrix);
    context->VSSetConstantBuffers(1, 1, buffer.GetAdressOfBuffer());

    // Bind verticies to VertexShader
    this->mesh->BindMeshBuffers(context);

    // Draw all submeshes
    for (size_t i = 0; i < this->mesh->GetNrOfSubMeshes(); i++) {
        this->mesh->PerformSubMeshDrawCall(context, i);
    }
}