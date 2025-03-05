#include "SceneObject.hpp"
#include <DirectXMath.h>
#include "ConstantBuffer.hpp"
#include <iostream>
namespace dx = DirectX;

SceneObject::SceneObject(Transform transform, Mesh* mesh) : transform(transform), mesh(mesh) {}

void SceneObject::Draw(ID3D11Device* device,  ID3D11DeviceContext* context) const {
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

void SceneObject::Update() { 
    // static float scale = 1;
    // this->transform.Rotate(0, 0.01, 0);
    // this->transform.Move(DirectX::XMVectorScale(this->transform.GetDirectionVector(), 0.1));
    // this->transform.SetScale(DirectX::XMVectorSet(scale, scale, scale, 1));
    // scale += 0.001;
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
