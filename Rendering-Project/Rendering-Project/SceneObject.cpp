#include "SceneObject.hpp"
#include <DirectXMath.h>
#include <iostream>
namespace dx = DirectX;

SceneObject::SceneObject(Transform transform, Mesh* mesh) : transform(transform), mesh(mesh) {
    this->boundingBox = mesh->GetBoundingBox();

    DirectX::XMFLOAT4X4 worldMatrix = this->GetWorldMatrix();
    DirectX::XMMATRIX matrix        = DirectX::XMLoadFloat4x4(&worldMatrix);
    this->boundingBox.Transform(this->boundingBox, matrix);
}

void SceneObject::InitBuffer(ID3D11Device* device) {
    DirectX::XMFLOAT4X4 matrix = this->GetWorldMatrix();
    this->matrixBuffer.Initialize(device, sizeof(matrix), &matrix);
}

void SceneObject::Update() {
    // static float scale = 1;
    // this->transform.Rotate(0, 0.01, 0);
    // this->transform.Move(DirectX::XMVectorScale(this->transform.GetDirectionVector(), 0.1));
    // this->transform.SetScale(DirectX::XMVectorSet(scale, scale, scale, 1));
    // scale += 0.001;
}

void SceneObject::SetBoundingBox(DirectX::BoundingBox& boundingBox) { this->boundingBox = boundingBox; }

DirectX::BoundingBox SceneObject::GetBoundingBox() const { return this->boundingBox; }

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