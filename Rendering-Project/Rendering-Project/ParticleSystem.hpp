#pragma once
#include "ParticleBuffer.hpp"
#include <d3d11.h>
#include <string>
#include <unordered_map>
#include <wrl/client.h>

class ParticleSystem {
  public:
    ParticleSystem();
    ~ParticleSystem() = default;

    HRESULT Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device, UINT size, UINT nrOf, bool dynamic, bool hasSRV,
                       bool hasUAV);
    HRESULT LoadShaders();
    ID3D11VertexShader* GetVertexShader() const;
    ID3D11PixelShader* GetPixelShader() const;
    ID3D11GeometryShader* GetGeometryShader() const;
    ID3D11ShaderResourceView* GetSRV() const;
    ID3D11UnorderedAccessView* GetUAV() const;
    UINT GetParticleCount() const;

  private:
    ParticleBuffer particleBuffer;
    std::unordered_map<std::string, std::wstring> shaderPaths;

    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometryShader;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> computeShader;
};
