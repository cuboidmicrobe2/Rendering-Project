#pragma once
#include "ParticleBuffer.hpp"
#include <d3d11.h>
#include <string>
#include <unordered_map>
#include <wrl/client.h>

struct Particle {
    float position[3];
    float velocity[3];
    float lifetime;
    float maxLifetime;
};

class ParticleSystem {
  public:
    ParticleSystem();
    ~ParticleSystem() = default;

    HRESULT Initialize(ID3D11Device* device, UINT size, UINT nrOf, bool dynamic, bool hasSRV, bool hasUAV);
    HRESULT InitializeParticles(ID3D11DeviceContext* immediateContext, UINT count = 100);

    HRESULT LoadShaders(ID3D11Device* device, ID3D11DeviceContext* immediateContext);
    void UpdateParticles(ID3D11Device* device, ID3D11DeviceContext* immediateContext, float deltaTime);
    void SetInputLayout(ID3D11DeviceContext* immediateContext);

    ID3D11ComputeShader* GetComputeShader() const;
    ID3D11VertexShader* GetVertexShader() const;
    ID3D11PixelShader* GetPixelShader() const;
    ID3D11GeometryShader* GetGeometryShader() const;
    ID3D11ShaderResourceView* GetSRV() const;
    ID3D11UnorderedAccessView* GetUAV() const;
    UINT GetParticleCount() const;

  private:
    ParticleBuffer particleBuffer;
    std::unordered_map<std::string, std::wstring> shaderPaths;
    bool isInitialized;

    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometryShader;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> computeShader;
};
