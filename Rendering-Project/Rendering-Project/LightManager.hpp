#ifndef LIGHT_MANAGER_HPP
#define LIGHT_MANAGER_HPP
#include "Light.hpp"
#include <d3d11_4.h>
#include <vector>
#include <wrl/client.h>
#include "StructuredBuffer.hpp"
#include "DirectionalLight.hpp"

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
    LightManager(UINT spotLightRes, UINT dirLightRes);
    HRESULT Init(ID3D11Device* device);

    void BindLightData(ID3D11DeviceContext* context, UINT SpotLightSlot, UINT DirLightSlot);
    void BindDepthTextures(ID3D11DeviceContext* context, UINT SpotLightSlot, UINT DirLightSlot);
    void UnbindDepthTextures(ID3D11DeviceContext* context, UINT SpotLightSlot, UINT DirLightSlot);

    ID3D11ShaderResourceView *const* GetAdressOfSpotlightDSSRV() const;
    const std::vector<Light>& GetSpotLights() const;
    const D3D11_VIEWPORT& GetSpotLightVP() const;
    void AddSpotLight(const Light& light);

    ID3D11ShaderResourceView* const* GetAdressOfDirlightDSSRV() const;
    void AddDirectionalLight(const DirectionalLight& light);
    const std::vector<DirectionalLight>& GetDirectionalLights() const;
    const D3D11_VIEWPORT& GetDirectionalLightVP() const;


  private:
    std::vector<LightData> GetSpotLightData() const;
    StructuredBuffer spotLightBuffer;
    D3D11_VIEWPORT spotLightViewPort;
    std::vector<Light> Spotlights;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> SpotLightDepthTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SpotLightSRV;

    std::vector<LightData> GetDirLightData() const;
    StructuredBuffer dirLightBuffer;
    D3D11_VIEWPORT directionLightViewPort;
    std::vector<DirectionalLight> directionalLights;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> dirLightDepthTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> DirLightSRV;

};

#endif // !LIGHT_MANAGER_HPP
