#ifndef LIGHT_HPP
#define LIGHT_HPP
#include "ConstantBuffer.hpp"
#include "Transform.hpp"
#include <wrl/client.h>

class Light {
  public:
    inline Light(Transform transform, DirectX::XMVECTOR color, float intensity,
                 float angle);
    ~Light() {};
    inline HRESULT Init(ID3D11Device* device, UINT resolution, D3D11_DEPTH_STENCIL_VIEW_DESC* desc,
                        ID3D11Texture2D* depthStencil);
    inline float GetIntesity() const { return this->intensity; }
    inline DirectX::XMVECTOR GetColor() const { return this->color; }
    Transform transform;
    inline DirectX::XMVECTOR GetDirection() const;
    inline float GetAngle() const;
    inline DirectX::XMMATRIX CreateViewMatrix() const;
    inline DirectX::XMMATRIX CreateProjectionMatrix() const;
    inline ID3D11DepthStencilView* GetDepthStencilVeiw() const;

  private:
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
    DirectX::XMVECTOR color;
    float angle;
    float intensity;

};

inline Light::Light(Transform transform, DirectX::XMVECTOR color, float intensity,
                    float angle)
    : transform(transform), color(color), intensity(intensity),
      angle(DirectX::XMConvertToRadians(angle)) {}

HRESULT Light::Init(ID3D11Device* device, UINT resolution, D3D11_DEPTH_STENCIL_VIEW_DESC* desc, ID3D11Texture2D* depthStencil) {
    return device->CreateDepthStencilView(depthStencil, desc, this->depthStencilView.GetAddressOf());
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

inline DirectX::XMVECTOR Light::GetDirection() const { return this->transform.GetDirectionVector(); }

inline float Light::GetAngle() const { return this->angle; }

#endif
