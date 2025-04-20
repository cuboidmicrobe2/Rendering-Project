#pragma once

#include "ConstantBuffer.hpp"
#include <d3d11_4.h>
#include <string>

class SubMesh {
  public:
    SubMesh()                                = default;
    ~SubMesh()                               = default;
    SubMesh(const SubMesh& other)            = default;
    SubMesh& operator=(const SubMesh& other) = default;
    SubMesh(SubMesh&& other)                 = default;
    SubMesh& operator=(SubMesh&& other)      = default;

    void Initialize(ID3D11Device* device, size_t startIndexValue, size_t nrOfIndicesInSubMesh,
                    ID3D11ShaderResourceView* ambientTextureSRV, ID3D11ShaderResourceView* diffuseTextureSRV,
                    ID3D11ShaderResourceView* specularTextureSRV, ID3D11ShaderResourceView* normalMapTexture,
                    ID3D11ShaderResourceView* parallaxMapTexture);

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

    ID3D11ShaderResourceView* ambientTexture   = nullptr;
    ID3D11ShaderResourceView* diffuseTexture   = nullptr;
    ID3D11ShaderResourceView* specularTexture  = nullptr;
    ID3D11ShaderResourceView* normalMapTexture = nullptr;
    ID3D11ShaderResourceView* parallaxMapTexture = nullptr;

    struct PSMetaData_t {
        int hasDiffuse;
        int hasAmbient;
        int hasSpecular;
        int hasNormal;
        int hasParallax;
        int padding[3];
    };

};