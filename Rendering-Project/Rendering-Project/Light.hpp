#ifndef LIGHT_HPP
#define LIGHT_HPP
#include "ConstantBuffer.hpp"
#include "Transform.hpp"

class Light {
  public:
    inline Light(Transform transform, DirectX::XMVECTOR color, float intensity, DirectX::XMVECTOR direction,
                 float angle);
    ~Light() {};
    inline HRESULT Init(ID3D11Device* device, UINT resolution);
    inline float GetIntesity() const { return this->intensity; }
    inline DirectX::XMVECTOR GetColor() const { return this->color; }
    Transform transform;
    inline DirectX::XMVECTOR GetDirection() const;
    inline float GetAngle() const;
    inline DirectX::XMMATRIX CreateViewMatrix() const;
    inline DirectX::XMMATRIX CreateProjectionMatrix() const;
    inline ID3D11DepthStencilView* GetDepthStencilVeiw() const;

  private:
    DirectX::XMVECTOR color;
    DirectX::XMVECTOR direction;
    float angle;
    float intensity;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencil;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
};

inline Light::Light(Transform transform, DirectX::XMVECTOR color, float intensity, DirectX::XMVECTOR direction,
                    float angle)
    : transform(transform), color(color), intensity(intensity), direction(direction),
      angle(DirectX::XMConvertToRadians(angle)) {}

HRESULT Light::Init(ID3D11Device* device, UINT resolution) {
    D3D11_TEXTURE2D_DESC depthStencilDesc = {
        depthStencilDesc.Width              = resolution,
        depthStencilDesc.Height             = resolution,
        depthStencilDesc.MipLevels          = 1,
        depthStencilDesc.ArraySize          = 1,
        depthStencilDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT,
        depthStencilDesc.SampleDesc.Count   = 1,
        depthStencilDesc.SampleDesc.Quality = 0,
        depthStencilDesc.Usage              = D3D11_USAGE_DEFAULT,
        depthStencilDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL,
        depthStencilDesc.CPUAccessFlags     = 0,
        depthStencilDesc.MiscFlags          = 0,
    };

    HRESULT result = device->CreateTexture2D(&depthStencilDesc, nullptr, this->depthStencil.GetAddressOf());
    if (FAILED(result)) return result;

    return device->CreateDepthStencilView(this->depthStencil.Get(), nullptr, this->depthStencilView.GetAddressOf());
}

inline DirectX::XMMATRIX Light::CreateViewMatrix() const {
    DirectX::XMVECTOR forward = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 0, 1, 0),
                                                         this->transform.GetRotationQuaternion()); // Local forward
    DirectX::XMVECTOR up =
        DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 1, 0, 0), this->transform.GetRotationQuaternion()); // Local up
    return DirectX::XMMatrixLookToLH(this->transform.GetPosition(), forward, up);
}

inline DirectX::XMMATRIX Light::CreateProjectionMatrix() const {
    return DirectX::XMMatrixPerspectiveFovLH(this->GetAngle(), 1, 1, 100);
}

inline ID3D11DepthStencilView* Light::GetDepthStencilVeiw() const { return this->depthStencilView.Get(); }

inline DirectX::XMVECTOR Light::GetDirection() const { return this->direction; }

inline float Light::GetAngle() const { return this->angle; }

#endif
