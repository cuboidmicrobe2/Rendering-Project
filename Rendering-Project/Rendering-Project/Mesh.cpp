#define STB_IMAGE_IMPLEMENTATION
#include "Mesh.hpp"
#include "OBJ_Loader.h"
#include "SimpleVertex.hpp"
#include "stb_image.h"
#include <DDSTextureLoader.h>
#include <DirectXCollision.h>
#include <WICTextureLoader.h>
#include <filesystem>
#include <fstream>
#include <wrl/client.h>

namespace fs = std::filesystem;
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> LoadNormal(ID3D11Device* device, const std::string& filename,
                                                            const std::string& filenameDisp);
void CreateDefaultTexture(ID3D11Device* device, ID3D11ShaderResourceView** viewOut);

Mesh::Mesh(ID3D11Device* device, const std::string& folderpath, const std::string& objname) {
    this->Initialize(device, folderpath, objname);
}

Mesh::Mesh(Mesh&& other) noexcept
    : subMeshes(std::move(other.subMeshes)), indexBuffer(std::move(other.indexBuffer)),
      vertexBuffer(std::move(other.vertexBuffer)), boundingBox(std::move(other.boundingBox)) {}

void Mesh::Initialize(ID3D11Device* device, const std::string& folderpath, const std::string& objname) {
    objl::Loader loader;

    std::string objpath = folderpath + "/" + objname;

    bool loaded = loader.LoadFile(objpath);

    if (loaded) {
        size_t meshStartIndex = 0;
        std::vector<uint32_t> tempIndexBuffer;
        std::vector<SimpleVertex> tempVertexBuffer;
        std::vector<DirectX::XMFLOAT3> vertexPositions; // For bounding boxes

        for (auto& mesh : loader.LoadedMeshes) {
            SubMesh submesh;
            Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ambientSrv  = nullptr;
            Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> diffuseSrv  = nullptr;
            Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> specularSrv = nullptr;
            Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalMap   = nullptr;
            float parallaxFactor                                         = 0;
            DirectX::XMFLOAT3 ambientFactor;
            DirectX::XMFLOAT3 diffuseFactor;
            DirectX::XMFLOAT3 specularFactor;
            float shininess = 100;
            Microsoft::WRL::ComPtr<ID3D11Resource> diffuseTexture;
            std::cout << mesh.MeshMaterial.map_Ka << "\n";

            // Load diffuse texture if there is one
            diffuseFactor = {mesh.MeshMaterial.Kd.X, mesh.MeshMaterial.Kd.Y, mesh.MeshMaterial.Kd.Z};
            if (!mesh.MeshMaterial.map_Kd.empty()) {
                std::cout << "Trying to bind map_Kd\n";
                std::string path = folderpath + "/" + mesh.MeshMaterial.map_Kd;
                std::wstring wpath(path.begin(), path.end());
                HRESULT createShaderResult = DirectX::CreateWICTextureFromFile(
                    device, wpath.c_str(), diffuseTexture.GetAddressOf(), diffuseSrv.GetAddressOf());

                if (FAILED(createShaderResult)) throw std::runtime_error("failed to load diffuse texture");
            } else {
                CreateDefaultTexture(device, diffuseSrv.GetAddressOf());
            }

            // Load ambient Texture if there is one
            ambientFactor = {mesh.MeshMaterial.Ka.X, mesh.MeshMaterial.Ka.Y, mesh.MeshMaterial.Ka.Z};
            if (!mesh.MeshMaterial.map_Ka.empty()) {
                std::cout << "Trying to bind map_Ka\n";
                std::string path = folderpath + "/" + mesh.MeshMaterial.map_Ka;
                std::wstring wpath(path.begin(), path.end());
                HRESULT createShaderResult =
                    DirectX::CreateWICTextureFromFile(device, wpath.c_str(), nullptr, ambientSrv.GetAddressOf());

                if (FAILED(createShaderResult)) {
                    std::cerr << createShaderResult << "\n";
                    throw std::runtime_error("failed to load ambient texture");
                }
            } else {
                ambientSrv = diffuseSrv;
            }

            // Load specular texture if there is one
            specularFactor = {mesh.MeshMaterial.Ks.X, mesh.MeshMaterial.Ks.Y, mesh.MeshMaterial.Ks.Z};
            shininess      = mesh.MeshMaterial.Ns;
            if (shininess == 0) shininess = 100;

            if (!mesh.MeshMaterial.map_Ks.empty()) {
                std::cout << "Trying to bind map_Ks\n";
                std::string path = folderpath + "/" + mesh.MeshMaterial.map_Ks;
                std::wstring wpath(path.begin(), path.end());
                HRESULT createShaderResult =
                    DirectX::CreateWICTextureFromFile(device, wpath.c_str(), nullptr, specularSrv.GetAddressOf());

                if (FAILED(createShaderResult)) throw std::runtime_error("failed to load specular texture");
            }

            // Load normal texture if there is one
            if (!mesh.MeshMaterial.map_bump.empty()) {
                std::cout << "Trying to bind normal map\n";
                std::string path = folderpath + "/" + mesh.MeshMaterial.map_bump;
                std::string dispPath;
                if (!mesh.MeshMaterial.map_d.empty()) {
                    dispPath       = folderpath + "/" + mesh.MeshMaterial.map_d;
                    parallaxFactor = 0.05f;
                }
                normalMap = LoadNormal(device, path, dispPath);
            }

            // Initialize and add submesh
            submesh.Initialize(device, meshStartIndex, mesh.Indices.size(), ambientSrv, diffuseSrv, specularSrv,
                               normalMap, parallaxFactor, ambientFactor, diffuseFactor, specularFactor, shininess);

            this->subMeshes.emplace_back(std::move(submesh));

            // Add indices to temp index buffer
            tempIndexBuffer.reserve(mesh.Indices.size());
            for (auto& indice : mesh.Indices) {
                tempIndexBuffer.emplace_back(indice + (unsigned int) meshStartIndex);
            }
            meshStartIndex += mesh.Indices.size();

            // Add vertexes to temp vertex buffer
            tempVertexBuffer.reserve(mesh.Vertices.size());
            for (const auto& vertex : mesh.Vertices) {
                tempVertexBuffer.emplace_back(vertex);
                vertexPositions.emplace_back(DirectX::XMFLOAT3(vertex.Position.X, vertex.Position.Y,
                                                               vertex.Position.Z)); // For bounding boxes
            }
        }

        // Initialize buffers
        this->vertexBuffer.Initialize(device, sizeof(SimpleVertex), (UINT) tempVertexBuffer.size(),
                                      tempVertexBuffer.data());
        this->indexBuffer.Initialize(device, tempIndexBuffer.size(), tempIndexBuffer.data());
        this->boundingBox.CreateFromPoints(this->boundingBox, vertexPositions.size(), vertexPositions.data(),
                                           sizeof(DirectX::XMFLOAT3));

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

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> LoadNormal(ID3D11Device* device, const std::string& filename,
                                                            const std::string& filenameDisp) {
    int width, height, channels;
    stbi_uc* normalData = stbi_load(filename.c_str(), &width, &height, &channels, 4);
    if (!normalData) {
        throw std::runtime_error("Failed to load normal map: " + filename);
    }

    if (!filenameDisp.empty()) {
        int dWidth, dHeight, dChannels;
        stbi_uc* dispData = stbi_load(filenameDisp.c_str(), &dWidth, &dHeight, &dChannels, 1);
        if (!dispData) {
            stbi_image_free(normalData);
            throw std::runtime_error("Failed to load displacement map: " + filenameDisp);
        }

        if (dWidth != width || dHeight != height) {
            stbi_image_free(normalData);
            stbi_image_free(dispData);
            throw std::runtime_error("Displacement map dimensions do not match normal map dimensions");
        }

        int pixelCount = width * height;
        for (int i = 0; i < pixelCount; ++i) {
            normalData[4 * i + 3] = dispData[i]; // Replace alpha with displacement
        }

        stbi_image_free(dispData);
    }

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width                = width;
    texDesc.Height               = height;
    texDesc.MipLevels            = 1;
    texDesc.ArraySize            = 1;
    texDesc.Format               = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count     = 1;
    texDesc.Usage                = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags            = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags       = 0;
    texDesc.MiscFlags            = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem                = normalData;
    initData.SysMemPitch            = width * 4;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    HRESULT hr = device->CreateTexture2D(&texDesc, &initData, &texture);
    stbi_image_free(normalData);

    if (FAILED(hr) || !texture) {
        throw std::runtime_error("Failed to create texture from image data");
    }

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
    hr = device->CreateShaderResourceView(texture.Get(), nullptr, &srv);

    if (FAILED(hr) || !srv) {
        throw std::runtime_error("Failed to create SRV from texture");
    }

    return srv; // caller is responsible for releasing
}

void CreateDefaultTexture(ID3D11Device* device, ID3D11ShaderResourceView** viewOut) {
    UINT32 colorData[4] = {
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
    };
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width                = 2;
    texDesc.Height               = 2;
    texDesc.MipLevels            = 1;
    texDesc.ArraySize            = 1;
    texDesc.Format               = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count     = 1;
    texDesc.Usage                = D3D11_USAGE_IMMUTABLE;
    texDesc.BindFlags            = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem                = colorData;
    initData.SysMemPitch            = 2 * sizeof(UINT32); // Row size: 2 texels

    ID3D11Texture2D* pTexture = nullptr;
    HRESULT hr                = device->CreateTexture2D(&texDesc, &initData, &pTexture);
    if (FAILED(hr)) {
        std::cerr << "Failed to create default texture (very bad) Error: " << hr << "\n";
        throw std::runtime_error("Couldn't create default texture");
    }

    hr = device->CreateShaderResourceView(pTexture, nullptr, viewOut);
    if (FAILED(hr)) {
        std::cerr << "Failed to create srv, Error: " << hr << "\n";
        throw std::runtime_error("Couldn't create srv");
    }
    pTexture->Release();
}

DirectX::BoundingBox Mesh::GetBoundingBox() const { return this->boundingBox; }
