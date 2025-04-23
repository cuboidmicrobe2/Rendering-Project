#pragma once

#include "Camera.hpp"
#include "Gbuffer.hpp"
#include "SceneObject.hpp"
#include "Transform.hpp"
#include <d3d11.h>

class DCEM : public SceneObject {
  public:
    DCEM(Transform transform, ID3D11PixelShader* normalPS, ID3D11PixelShader* DCEMPS, Mesh* mesh, UINT size);
    inline ~DCEM() override {}
    void Init(ID3D11Device* device) override;
    void Draw(ID3D11Device* device, ID3D11DeviceContext* context) override;
    void Update() override;
    std::array<Camera, 6>& GetCameras();

  private:
    std::array<Camera, 6> cameras;
    UINT size;
    RenderingResources rr;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    ID3D11PixelShader* PS;
    ID3D11PixelShader* normalPS;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
};