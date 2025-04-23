#include "SubMesh.hpp"
#include <iostream>

SubMesh::SubMesh(SubMesh&& other) noexcept {
    this->startIndex       = std::move(other.startIndex);
    this->nrOfIndices      = std::move(other.nrOfIndices);
    this->ambientTexture   = std::move(other.ambientTexture);
    this->diffuseTexture   = std::move(other.diffuseTexture);
    this->specularTexture  = std::move(other.specularTexture);
    this->normalMapTexture = std::move(other.normalMapTexture);
    this->PSMetaData       = std::move(other.PSMetaData);
}

void SubMesh::Initialize(ID3D11Device* device, size_t startIndexValue, size_t nrOfIndicesInSubMesh,
                         ID3D11ShaderResourceView* ambientTextureSRV, ID3D11ShaderResourceView* diffuseTextureSRV,
                         ID3D11ShaderResourceView* specularTextureSRV,
                         ID3D11ShaderResourceView* normalMapTexture, float  parallaxFactor) {
    this->startIndex       = startIndexValue;
    this->nrOfIndices      = nrOfIndicesInSubMesh;
    this->ambientTexture   = ambientTextureSRV;
    this->diffuseTexture   = diffuseTextureSRV;
    this->specularTexture  = specularTextureSRV;
    this->normalMapTexture = normalMapTexture;

    PSMetaData_t metaData{
        (bool)diffuseTextureSRV,
        (bool)ambientTextureSRV,
        (bool)specularTextureSRV,
        (bool)normalMapTexture,
        parallaxFactor,
    };

    this->PSMetaData.Initialize(device, sizeof(PSMetaData_t), &metaData);
}

void SubMesh::PerformDrawCall(ID3D11DeviceContext* context) {
    context->PSSetConstantBuffers(0, 1, this->PSMetaData.GetAdressOfBuffer());
    context->PSSetShaderResources(0, 1, this->diffuseTexture.GetAddressOf());
    context->PSSetShaderResources(1, 1, this->ambientTexture.GetAddressOf());
    context->PSSetShaderResources(2, 1, this->specularTexture.GetAddressOf());
    context->PSSetShaderResources(3, 1, this->normalMapTexture.GetAddressOf());

    context->DrawIndexed(this->nrOfIndices, this->startIndex, 0);
}

ID3D11ShaderResourceView* SubMesh::GetAmbientSRV() const { return this->ambientTexture.Get(); }

ID3D11ShaderResourceView* SubMesh::GetDiffuseSRV() const { return this->diffuseTexture.Get(); }

ID3D11ShaderResourceView* SubMesh::GetSpecularSRV() const { return this->specularTexture.Get(); }

ID3D11ShaderResourceView* SubMesh::GetNormalMapSRV() const { return this->normalMapTexture.Get(); }

ID3D11ShaderResourceView* SubMesh::GetParallaxMapSRV() const { return this->parallaxMapTexture.Get(); }
