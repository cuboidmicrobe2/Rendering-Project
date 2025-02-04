#include "Mesh.hpp"

void Mesh::Initialize(ID3D11Device* device, const MeshData& meshInfo) {
    this->subMeshes.reserve(meshInfo.subMeshInfo.size());
    for (const MeshData::SubMeshInfo& subMeshInfo : meshInfo.subMeshInfo) {
        SubMesh newSubMesh;
        newSubMesh.Initialize(subMeshInfo.startIndexValue, subMeshInfo.nrOfIndicesInSubMesh,
                              subMeshInfo.ambientTextureSRV, subMeshInfo.diffuseTextureSRV,
                              subMeshInfo.specularTextureSRV);
        this->subMeshes.emplace_back(newSubMesh);
    }

    this->vertexBuffer.Initialize(device, static_cast<UINT>(meshInfo.vertexInfo.sizeOfVertex),
                                  static_cast<UINT>(meshInfo.vertexInfo.nrOfVerticesInBuffer),
                                  meshInfo.vertexInfo.vertexData);

    this->indexBuffer.Initialize(device, meshInfo.indexInfo.nrOfIndicesInBuffer, meshInfo.indexInfo.indexData);
}

void Mesh::BindMeshBuffers(ID3D11DeviceContext* context) const {
    // Bind the vertex buffer
    UINT stride                = this->vertexBuffer.GetVertexSize();
    UINT offset                = 0;
    ID3D11Buffer* vertexBuffer = this->vertexBuffer.GetBuffer();
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    // Bind the index buffer
    ID3D11Buffer* indexBuffer = this->indexBuffer.GetBuffer();
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
}

void Mesh::PerformSubMeshDrawCall(ID3D11DeviceContext* context, size_t subMeshIndex) const {
    this->subMeshes.at(subMeshIndex).PerformDrawCall(context);
}

size_t Mesh::GetNrOfSubMeshes() const { return this->subMeshes.size(); }

ID3D11ShaderResourceView* Mesh::GetAmbientSRV(size_t subMeshIndex) const {
    return this->subMeshes.at(subMeshIndex).GetAmbientSRV();
}

ID3D11ShaderResourceView* Mesh::GetDiffuseSRV(size_t subMeshIndex) const {
    return this->subMeshes.at(subMeshIndex).GetDiffuseSRV();
}

ID3D11ShaderResourceView* Mesh::GetSpecularSRV(size_t subMeshIndex) const {
    return this->subMeshes.at(subMeshIndex).GetSpecularSRV();
}
