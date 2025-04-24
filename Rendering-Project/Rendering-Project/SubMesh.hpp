#pragma once

#include "ConstantBuffer.hpp"
#include <DirectXMath.h>
#include <d3d11_4.h>
#include <string>
#include <wrl/client.h>

class SubMesh {
  public:
    SubMesh() = default;
    ~SubMesh() {

    };
    SubMesh(const SubMesh& other)            = delete;
    SubMesh& operator=(const SubMesh& other) = delete;
    SubMesh(SubMesh&& other) noexcept;
    SubMesh& operator=(SubMesh&& other) = default;

    void Initialize(ID3D11Device* device, size_t startIndexValue, size_t nrOfIndicesInSubMesh,
                    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ambientTextureSRV,
                    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> diffuseTextureSRV,
                    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> specularTextureSRV,
                    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalMapTexture, float parallaxFactor,
                    DirectX::XMFLOAT3 ambientFactor, DirectX::XMFLOAT3 diffuseFactor, DirectX::XMFLOAT3 specularFactor,
                    int shininess);

    void PerformDrawCall(ID3D11DeviceContext* context);

    ConstantBuffer PSMetaData;

    ID3D11ShaderResourceView* GetAmbientSRV() const;
    ID3D11ShaderResourceView* GetDiffuseSRV() const;
    ID3D11ShaderResourceView* GetSpecularSRV() const;
    ID3D11ShaderResourceView* GetNormalMapSRV() const;
    ID3D11ShaderResourceView* GetParallaxMapSRV() const;

  private:
    size_t startIndex  = 0;
    size_t nrOfIndices = 0;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ambientTexture     = nullptr;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> diffuseTexture     = nullptr;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> specularTexture    = nullptr;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalMapTexture   = nullptr;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> parallaxMapTexture = nullptr;

    struct PSMetaData_t {
        DirectX::XMFLOAT3 diffuseFactor;
        int hasDiffuseMap;
        DirectX::XMFLOAT3 ambientFactor;
        int hasAmbientMap;
        DirectX::XMFLOAT3 specularFactor;
        int hasSpecularMap;
        int hasNormal;
        float parallaxFactor;
        int shininess;
        int padding;
    };
};