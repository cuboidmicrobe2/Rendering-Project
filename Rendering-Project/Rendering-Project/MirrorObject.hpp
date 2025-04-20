#pragma once

#include "Camera.hpp"
#include "SceneObject.hpp"
#include "Transform.hpp"
#include <d3d11.h>

class MirrorObject : public SceneObject {
  public:
    MirrorObject(Transform transform, UINT width, UINT height);
    void Draw(ID3D11Device* device, ID3D11DeviceContext* context) const override;
    void Update() override;
    void Init(ID3D11Device* device);

  private:
    Mesh mesh;
    Camera camera;
    ID3D11Texture2D* texture;
    ID3D11ShaderResourceView* srv;
    ID3D11RenderTargetView* rtv;
};

MirrorObject::MirrorObject(Transform transform, UINT width, UINT height)
    : SceneObject(transform),
      camera(90, 16.f / 9.f, 1, 1000, transform.GetPosition(), transform.GetRotationQuaternion(), nullptr,
             D3D11_VIEWPORT{
                 .TopLeftX = 0,
                 .TopLeftY = 0,
                 .Width    = static_cast<FLOAT>(width),
                 .Height   = static_cast<FLOAT>(height),
                 .MinDepth = 0.0f,
                 .MaxDepth = 1.0f,
             }) {}

void MirrorObject::Draw(ID3D11Device* device, ID3D11DeviceContext* context) const {}

void MirrorObject::Update() {}

inline void MirrorObject::Init(ID3D11Device* device) {
    D3D11_TEXTURE2D_DESC desc{
        .Width     = this->camera.GetViewPort().Width,
        .Height    = this->camera.GetViewPort().Height,
        .MipLevels = 1,
        .ArraySize = 6,
        .Format    = DXGI_FORMAT_R8G8B8A8_UNORM,
        .SampleDesc{
            .Count   = 1,
            .Quality = 0,
        },
        .BindFlags      = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
        .CPUAccessFlags = 0,
        .MiscFlags      = D3D11_RESOURCE_MISC_TEXTURECUBE,
    };
    HRESULT hr = device->CreateTexture2D(&desc, nullptr, &this->texture);

    D3D11_UNORDERED_ACCESS_VIEW_DESC descUAV{
        .Format        = DXGI_FORMAT_R8G8B8A8_UNORM,
        .ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D,
        .Texture2D{
            .MipSlice = 0,
        },
    };
    device->CreateUnorderedAccessView(texture, &descUAV, this->camera.GetAdressOfUAV());
}
