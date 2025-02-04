#include "SubMesh.hpp"

void SubMesh::Initialize(size_t startIndexValue, size_t nrOfIndicesInSubMesh,
                         ID3D11ShaderResourceView* ambientTextureSRV, ID3D11ShaderResourceView* diffuseTextureSRV,
                         ID3D11ShaderResourceView* specularTextureSRV) {}

void SubMesh::PerformDrawCall(ID3D11DeviceContext* context) const {
    context->DrawIndexed(this->nrOfIndices, this->startIndex, 0);
}

ID3D11ShaderResourceView* SubMesh::GetAmbientSRV() const { return this->ambientTexture; }

ID3D11ShaderResourceView* SubMesh::GetDiffuseSRV() const { return this->diffuseTexture; }

ID3D11ShaderResourceView* SubMesh::GetSpecularSRV() const { return this->specularTexture; }
