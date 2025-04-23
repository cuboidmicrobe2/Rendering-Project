#include "SimpleObject.hpp"

SimpleObject::SimpleObject(Transform transform, Mesh* mesh) : SceneObject(transform, mesh) {}

void SimpleObject::Draw(ID3D11Device* device, ID3D11DeviceContext* context) {
    // Bind verticies to VertexShader
    this->mesh->BindMeshBuffers(context);

    // Draw all submeshes
    for (size_t i = 0; i < this->mesh->GetNrOfSubMeshes(); i++) {
        this->mesh->PerformSubMeshDrawCall(context, i);
    }
}

void SimpleObject::Init(ID3D11Device* device) {}
