#ifndef LIGHT_MANAGER_HPP
#define LIGHT_MANAGER_HPP
#include "Light.hpp"
#include <d3d11_4.h>
#include <vector>
#include <wrl/client.h>
#include "StructuredBuffer.hpp"

struct LightData {
    float pos[3];
    float intensity;
    float color[4];
    float direction[3];
    float angle;
    DirectX::XMFLOAT4X4 viewProjectionMatrix;
};

class LightManager {
  public:
    LightManager(UINT resoulution);
    HRESULT Init(ID3D11Device* device);
    ID3D11ShaderResourceView *const* GetAdressOfDSSRV() const;
    const std::vector<Light>& GetLights() const;
    void AddLight(const Light& light);
    const D3D11_VIEWPORT& GetViewPort() const;
    void BindLightData(ID3D11DeviceContext* context, UINT slot);
    void BindDepthTextures(ID3D11DeviceContext* context, UINT slot);
    void UnbindDepthTextures(ID3D11DeviceContext* context, UINT slot);

  private:
    std::vector<LightData> GetRawLightData() const;
    StructuredBuffer lightBuffer;
    D3D11_VIEWPORT viewport;
    UINT resoulution;
    std::vector<Light> Spotlights;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencils;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
};

#endif // !LIGHT_MANAGER_HPP
