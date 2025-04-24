#include "SceneObject.hpp"
#include <DirectXMath.h>
#include <iostream>
namespace dx = DirectX;

SceneObject::SceneObject(Transform transform, Mesh* mesh, bool shouldBeTesselated, bool showTessellation)
    : transform(transform), mesh(mesh), boundingBox(mesh->GetBoundingBox()), shouldBeTesselated(shouldBeTesselated),
      showTessellation(showTessellation) {
    DirectX::XMMATRIX scaleMatrix       = DirectX::XMMatrixScalingFromVector(this->transform.GetScale());
    DirectX::XMMATRIX rotationMatrix    = DirectX::XMMatrixRotationQuaternion(this->transform.GetRotationQuaternion());
    DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslationFromVector(this->transform.GetPosition());

    DirectX::XMMATRIX matrix = scaleMatrix * rotationMatrix * translationMatrix;

    this->boundingBox.Transform(this->boundingBox, matrix);
}

SceneObject::~SceneObject() {}

void SceneObject::Update() {}

void SceneObject::SetBoundingBox(DirectX::BoundingBox& boundingBox) { this->boundingBox = boundingBox; }

DirectX::BoundingBox SceneObject::GetBoundingBox() const { return this->boundingBox; }

bool SceneObject::GetTesselationValue() const { return this->shouldBeTesselated; }

bool SceneObject::GetTessellationVisibility() const { return this->showTessellation; }

void SceneObject::DrawMesh(ID3D11DeviceContext* context) {
    // Bind verticies to VertexShader
    this->mesh->BindMeshBuffers(context);

    // Draw all submeshes
    for (size_t i = 0; i < this->mesh->GetNrOfSubMeshes(); i++) {
        this->mesh->PerformSubMeshDrawCall(context, i);
    }
}

DirectX::XMFLOAT4X4 SceneObject::GetWorldMatrix() const {
    // Create the scaling, rotation, and translation matrices
    DirectX::XMMATRIX scaleMatrix       = DirectX::XMMatrixScalingFromVector(this->transform.GetScale());
    DirectX::XMMATRIX rotationMatrix    = DirectX::XMMatrixRotationQuaternion(this->transform.GetRotationQuaternion());
    DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslationFromVector(this->transform.GetPosition());

    // Combine the matrices to create the world matrix (scale * rotation * translation)
    DirectX::XMMATRIX worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

    // Transpose the matrix if needed (depends on the target platform/GPU conventions)
    worldMatrix = DirectX::XMMatrixTranspose(worldMatrix);

    // Store the result in a XMFLOAT4X4
    DirectX::XMFLOAT4X4 worldMatrixFloat4x4;
    DirectX::XMStoreFloat4x4(&worldMatrixFloat4x4, worldMatrix);

    return worldMatrixFloat4x4;
}