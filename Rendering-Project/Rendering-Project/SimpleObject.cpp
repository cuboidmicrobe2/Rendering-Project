#include "SimpleObject.hpp"

SimpleObject::SimpleObject(Transform transform, Mesh* mesh) : SceneObject(transform, mesh) {}

void SimpleObject::Draw(ID3D11Device* device, ID3D11DeviceContext* context) {
    // Write Object Worldmatrix to vertexShader
    DirectX::XMFLOAT4X4 worldMatrix = this->GetWorldMatrix();
    this->matrixBuffer.UpdateBuffer(context, &worldMatrix);
    context->VSSetConstantBuffers(1, 1, this->matrixBuffer.GetAdressOfBuffer());

    // Bind verticies to VertexShader
    this->mesh->BindMeshBuffers(context);

    // Draw all submeshes
    for (size_t i = 0; i < this->mesh->GetNrOfSubMeshes(); i++) {
        this->mesh->PerformSubMeshDrawCall(context, i);
    }
}

void SimpleObject::Init(ID3D11Device* device) {
    DirectX::XMFLOAT4X4 matrix = this->GetWorldMatrix();
    this->matrixBuffer.Initialize(device, sizeof(matrix), &matrix);
}
