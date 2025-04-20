#include "SubMesh.hpp"
#include <iostream>

void SubMesh::Initialize(ID3D11Device* device, size_t startIndexValue, size_t nrOfIndicesInSubMesh,
                         ID3D11ShaderResourceView* ambientTextureSRV, ID3D11ShaderResourceView* diffuseTextureSRV,
                         ID3D11ShaderResourceView* specularTextureSRV,
                         ID3D11ShaderResourceView* normalMapTexture, ID3D11ShaderResourceView* parallaxMapTexture) {
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
        (bool)parallaxMapTexture,
    };

    this->PSMetaData.Initialize(device, sizeof(PSMetaData_t), &metaData);
}

void SubMesh::PerformDrawCall(ID3D11DeviceContext* context) {
    context->PSSetConstantBuffers(0, 1, this->PSMetaData.GetAdressOfBuffer());
    context->PSSetShaderResources(0, 1, &this->diffuseTexture);
    context->PSSetShaderResources(1, 1, &this->ambientTexture);
    context->PSSetShaderResources(2, 1, &this->specularTexture);
    context->PSSetShaderResources(3, 1, &this->normalMapTexture);

    context->DrawIndexed(this->nrOfIndices, this->startIndex, 0);
}

ID3D11ShaderResourceView* SubMesh::GetAmbientSRV() const { return this->ambientTexture; }

ID3D11ShaderResourceView* SubMesh::GetDiffuseSRV() const { return this->diffuseTexture; }

ID3D11ShaderResourceView* SubMesh::GetSpecularSRV() const { return this->specularTexture; }

ID3D11ShaderResourceView* SubMesh::GetNormalMapSRV() const { return this->normalMapTexture; }

ID3D11ShaderResourceView* SubMesh::GetParallaxMapSRV() const { return this->parallaxMapTexture; }
