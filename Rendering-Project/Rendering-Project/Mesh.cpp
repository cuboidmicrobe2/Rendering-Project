#define STB_IMAGE_IMPLEMENTATION
#include "Mesh.hpp"
#include "OBJ_Loader.h"
#include "SimpleVertex.hpp"
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include <filesystem>
#include <fstream>
#include "stb_image.h"

namespace fs = std::filesystem;
ID3D11ShaderResourceView* LoadNormal(ID3D11Device* device, const std::string& filename);

Mesh::Mesh(ID3D11Device* device, const std::string& folderpath, const std::string& objname) {
    this->Initialize(device, folderpath, objname);
}

void Mesh::Initialize(ID3D11Device* device, const MeshData& meshInfo) {
    this->subMeshes.reserve(meshInfo.subMeshInfo.size());
    for (const MeshData::SubMeshInfo& subMeshInfo : meshInfo.subMeshInfo) {
        SubMesh newSubMesh;
        newSubMesh.Initialize(device, subMeshInfo.startIndexValue, subMeshInfo.nrOfIndicesInSubMesh,
                              subMeshInfo.ambientTextureSRV, subMeshInfo.diffuseTextureSRV,
                              subMeshInfo.specularTextureSRV, subMeshInfo.normalMapSRV, subMeshInfo.parallaxMapSRV);
        this->subMeshes.emplace_back(std::move(newSubMesh));
    }

    this->vertexBuffer.Initialize(device, static_cast<UINT>(meshInfo.vertexInfo.sizeOfVertex),
                                  static_cast<UINT>(meshInfo.vertexInfo.nrOfVerticesInBuffer),
                                  meshInfo.vertexInfo.vertexData);

    this->indexBuffer.Initialize(device, meshInfo.indexInfo.nrOfIndicesInBuffer, meshInfo.indexInfo.indexData);
}

void Mesh::Initialize(ID3D11Device* device, const std::string& folderpath, const std::string& objname) {
    objl::Loader loader;

    std::string objpath = folderpath + "/" + objname;

    bool loaded = loader.LoadFile(objpath);

    if (loaded) {
        size_t meshStartIndex = 0;
        std::vector<uint32_t> tempIndexBuffer;
        std::vector<SimpleVertex> tempVertexBuffer;
        for (auto& mesh : loader.LoadedMeshes) {
            SubMesh submesh;
            ID3D11ShaderResourceView* ambientSrv  = nullptr;
            ID3D11ShaderResourceView* diffuseSrv  = nullptr;
            ID3D11ShaderResourceView* specularSrv = nullptr;
            ID3D11ShaderResourceView* normalMap   = nullptr;
            ID3D11ShaderResourceView* parallaxMap = nullptr;

            std::cout << mesh.MeshMaterial.map_Ka << "\n";

            // Load ambient Texture if there is one
            if (!mesh.MeshMaterial.map_Ka.empty()) {
                std::cout << "Trying to bind map_Ka\n";
                std::string path = folderpath + "/" + mesh.MeshMaterial.map_Ka;
                std::wstring wpath(path.begin(), path.end());
                HRESULT createShaderResult =
                    DirectX::CreateWICTextureFromFile(device, wpath.c_str(), nullptr, &ambientSrv);

                if (FAILED(createShaderResult)) {
                    std::cerr << createShaderResult << "\n";
                    throw std::runtime_error("failed to load ambient texture");
                }
            }

            // Load diffuse texture if there is one
            if (!mesh.MeshMaterial.map_Kd.empty()) {
                std::cout << "Trying to bind map_Kd\n";
                std::string path = folderpath + "/" + mesh.MeshMaterial.map_Kd;
                std::wstring wpath(path.begin(), path.end());
                HRESULT createShaderResult =
                    DirectX::CreateWICTextureFromFile(device, wpath.c_str(), nullptr, &diffuseSrv);

                if (FAILED(createShaderResult)) throw std::runtime_error("failed to load diffuse texture");
            }

            // Load specular texture if there is one
            if (!mesh.MeshMaterial.map_Ks.empty()) {
                std::cout << "Trying to bind map_Ks\n";
                std::string path = folderpath + "/" + mesh.MeshMaterial.map_Ks;
                std::wstring wpath(path.begin(), path.end());
                HRESULT createShaderResult =
                    DirectX::CreateWICTextureFromFile(device, wpath.c_str(), nullptr, &specularSrv);

                if (FAILED(createShaderResult)) throw std::runtime_error("failed to load specular texture");
            }

            // Load normal texture if there is one
            if (!mesh.MeshMaterial.map_bump.empty()) {
                std::cout << "Trying to bind normal map\n";
                std::string path = folderpath + "/" + mesh.MeshMaterial.map_bump;
                //std::wstring wpath(path.begin(), path.end());
                normalMap = LoadNormal(device, path);
                //HRESULT createShaderResult =
                //    DirectX::CreateDDSTextureFromFile(device, wpath.c_str(), nullptr, &normalMap);

                //if (FAILED(createShaderResult)) {
                //    std::cerr << "Failed to load normal map, Error: "  << createShaderResult << "\n";
                //    throw std::runtime_error("failed to load normal map");
                //}
            }

            // Initialize and add submesh
            submesh.Initialize(device, meshStartIndex, mesh.Indices.size(), ambientSrv, diffuseSrv, specularSrv,
                               normalMap, parallaxMap);
            this->subMeshes.emplace_back(std::move(submesh));

            // Add indices to temp index buffer
            tempIndexBuffer.reserve(mesh.Indices.size());
            for (auto& indice : mesh.Indices) {
                tempIndexBuffer.emplace_back(indice + meshStartIndex);
            }
            meshStartIndex += mesh.Indices.size();

            // Add vertexes to temp vertex buffer
            tempVertexBuffer.reserve(mesh.Vertices.size());
            for (const auto& vertex : mesh.Vertices) {
                tempVertexBuffer.emplace_back(vertex);
            }
        }

        // Initialize buffers
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

void Mesh::PerformSubMeshDrawCall(ID3D11DeviceContext* context, size_t subMeshIndex) {
    this->subMeshes.at(subMeshIndex).PerformDrawCall(context);
}

size_t Mesh::GetNrOfSubMeshes() const { return this->subMeshes.size(); }

const std::vector<SubMesh>& Mesh::GetSubMeshes() const { return this->subMeshes; }

ID3D11ShaderResourceView* Mesh::GetAmbientSRV(size_t subMeshIndex) const {
    return this->subMeshes.at(subMeshIndex).GetAmbientSRV();
}

ID3D11ShaderResourceView* Mesh::GetDiffuseSRV(size_t subMeshIndex) const {
    return this->subMeshes.at(subMeshIndex).GetDiffuseSRV();
}

ID3D11ShaderResourceView* Mesh::GetSpecularSRV(size_t subMeshIndex) const {
    return this->subMeshes.at(subMeshIndex).GetSpecularSRV();
}

ID3D11ShaderResourceView* Mesh::GetNormalMapSRV(size_t subMeshIndex) const {
    return this->subMeshes.at(subMeshIndex).GetNormalMapSRV();
}

ID3D11ShaderResourceView* Mesh::GetParallaxMapSRV(size_t subMeshIndex) const {
    return this->subMeshes.at(subMeshIndex).GetParallaxMapSRV();
}

ID3D11ShaderResourceView* LoadNormal(ID3D11Device* device,
                                                  const std::string& filename) {
    int width, height, channels;
    stbi_uc* imageData = stbi_load(filename.c_str(), &width, &height, &channels, 4); // force RGBA
    if (!imageData) {
        throw std::runtime_error("Failed to load image via stb_image: " + filename);
    }

    // Describe the texture
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width                = width;
    texDesc.Height               = height;
    texDesc.MipLevels            = 1;
    texDesc.ArraySize            = 1;
    texDesc.Format               = DXGI_FORMAT_R8G8B8A8_UNORM; // important: UNORM, NOT SRGB
    texDesc.SampleDesc.Count     = 1;
    texDesc.Usage                = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags            = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags       = 0;
    texDesc.MiscFlags            = 0;

    // Describe initial data
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem                = imageData;
    initData.SysMemPitch            = width * 4;
    initData.SysMemSlicePitch       = 0;

    // Create texture
    ID3D11Texture2D* texture = nullptr;
    HRESULT hr               = device->CreateTexture2D(&texDesc, &initData, &texture);
    stbi_image_free(imageData); // free image data after creating the texture

    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create texture from raw image data");
    }

    // Create SRV
    ID3D11ShaderResourceView* srv = nullptr;
    hr                            = device->CreateShaderResourceView(texture, nullptr, &srv);
    texture->Release();

    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create SRV from normal map texture");
    }

    return srv;
}
