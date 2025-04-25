#ifndef DIRECTIONAL_LIGHT_HPP
#define DIRECTIONAL_LIGHT_HPP
#include "ConstantBuffer.hpp"
#include "Transform.hpp"
#include <wrl/client.h>

class DirectionalLight {
  public:
    static const int distanceFromTarget = 100;
    inline DirectionalLight(Transform transform, DirectX::XMVECTOR color, float intensity, float width, float height);
    ~DirectionalLight() {};
    inline HRESULT Init(ID3D11Device* device, UINT resolution, D3D11_DEPTH_STENCIL_VIEW_DESC* desc,
                        ID3D11Texture2D* depthStencil);
    inline float GetIntesity() const { return this->intensity; }
    inline DirectX::XMVECTOR GetColor() const { return this->color; }
    Transform transform;
    inline DirectX::XMVECTOR GetDirection() const;
    inline DirectX::XMMATRIX CreateViewMatrix() const;
    inline DirectX::XMMATRIX CreateProjectionMatrix() const;
    inline ID3D11DepthStencilView* GetDepthStencilVeiw() const;

  private:
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
    DirectX::XMVECTOR color;
    float intensity;
    float width;
    float height;
};

inline DirectionalLight::DirectionalLight(Transform transform, DirectX::XMVECTOR color, float intensity, float width,
                                          float height)
    : transform(transform),
      color(color), intensity(intensity), width(width), height(height) {

    DirectX::XMVECTOR pos =
        DirectX::XMVectorSubtract(transform.GetPosition(), DirectX::XMVectorScale(transform.GetDirectionVector(), this->distanceFromTarget));
    this->transform.SetPosition(pos);
}

HRESULT DirectionalLight::Init(ID3D11Device* device, UINT resolution, D3D11_DEPTH_STENCIL_VIEW_DESC* desc,
                    ID3D11Texture2D* depthStencil) {
    return device->CreateDepthStencilView(depthStencil, desc, this->depthStencilView.GetAddressOf());
}

inline DirectX::XMMATRIX DirectionalLight::CreateViewMatrix() const {
    DirectX::XMVECTOR forward = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 0, 1, 0),
                                                         this->transform.GetRotationQuaternion()); // Local forward
    DirectX::XMVECTOR up =
        DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 1, 0, 0), this->transform.GetRotationQuaternion()); // Local up
    return DirectX::XMMatrixLookToLH(this->transform.GetPosition(), forward, up);
}

inline DirectX::XMMATRIX DirectionalLight::CreateProjectionMatrix() const {
    return DirectX::XMMatrixOrthographicLH(this->width, this->height, 0.1f, 200.f);
}

inline ID3D11DepthStencilView* DirectionalLight::GetDepthStencilVeiw() const { return this->depthStencilView.Get(); }

inline DirectX::XMVECTOR DirectionalLight::GetDirection() const { return this->transform.GetDirectionVector(); }

#endif
