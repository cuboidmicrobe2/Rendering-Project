#include "Mesh.hpp"
#include "OBJ_Loader.h"
#include "SimpleVertex.hpp"
#include <WICTextureLoader.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

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

void Mesh::Initialize(ID3D11Device* device, std::string texturepath) {
    objl::Loader loader;
    bool loaded = loader.LoadFile(texturepath);

    if (loaded) {
        size_t meshStartIndex = 0;
        std::vector<uint32_t> tempIndexBuffer;
        std::vector<SimpleVertex> tempVertexBuffer;
        for (auto& mesh : loader.LoadedMeshes) {
            SubMesh submesh;
            ID3D11ShaderResourceView* ambientSrv  = nullptr;
            ID3D11ShaderResourceView* diffuseSrv  = nullptr;
            ID3D11ShaderResourceView* specularSrv = nullptr;

            if (!mesh.MeshMaterial.map_Ka.empty()) {
                std::string path           = texturepath + "/" + mesh.MeshMaterial.map_Ka;
                HRESULT createShaderResult = DirectX::CreateWICTextureFromFile(
                    device, std::wstring(path.begin(), path.end()).c_str(), nullptr, &ambientSrv);
                if (FAILED(createShaderResult)) throw std::runtime_error("failed to load ambient texture");
            } else {
                DirectX::CreateWICTextureFromFile(device, L"slimepfp.jpg", nullptr, &ambientSrv);
            }
            if (!mesh.MeshMaterial.map_Kd.empty()) {
                std::string path           = texturepath + "/" + mesh.MeshMaterial.map_Kd;
                HRESULT createShaderResult = DirectX::CreateWICTextureFromFile(
                    device, std::wstring(path.begin(), path.end()).c_str(), nullptr, &diffuseSrv);
                if (FAILED(createShaderResult)) throw std::runtime_error("failed to load diffuse texture");
            } else {
                DirectX::CreateWICTextureFromFile(device, L"slimepfp.jpg", nullptr, &diffuseSrv);
            }
            if (!mesh.MeshMaterial.map_Ka.empty()) {
                std::string path           = texturepath + "/" + mesh.MeshMaterial.map_Ka;
                HRESULT createShaderResult = DirectX::CreateWICTextureFromFile(
                    device, std::wstring(path.begin(), path.end()).c_str(), nullptr, &specularSrv);
                if (FAILED(createShaderResult)) throw std::runtime_error("failed to load specular texture");
            } else {
                DirectX::CreateWICTextureFromFile(device, L"slimepfp.jpg", nullptr, &specularSrv);
            }
            std::cout << ambientSrv << " " << diffuseSrv << " " << specularSrv << "\n";
            submesh.Initialize(meshStartIndex, mesh.Indices.size(), ambientSrv, diffuseSrv, specularSrv);
            this->subMeshes.emplace_back(submesh);

            // Add indices to temp index buffer
            meshStartIndex += mesh.Indices.size();
            for (auto& indice : mesh.Indices) {
                tempIndexBuffer.emplace_back(indice + meshStartIndex);
            }

            // Add vertexes to temp vertex buffer
            tempIndexBuffer.reserve(mesh.Vertices.size());
            for (const auto& vertex : mesh.Vertices) {
                tempVertexBuffer.emplace_back(SimpleVertex(vertex));
            }
        }
        this->vertexBuffer.Initialize(device, sizeof(SimpleVertex), tempVertexBuffer.size(), tempVertexBuffer.data());
        this->indexBuffer.Initialize(device, tempIndexBuffer.size(), tempIndexBuffer.data());
    } else {
        throw std::runtime_error("Failed to load model");
    }
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
