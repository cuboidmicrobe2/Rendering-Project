#ifndef CAMERA_HPP
#define CAMERA_HPP
#include "Gbuffer.hpp"
#include "InputHandler.hpp"
#include "Transform.hpp"
#include "directxmath.h"
#include <algorithm>
#include <array>
#include <d3d11_4.h>
#include <wrl/client.h>

class RenderingResources {
  public:
    RenderingResources() : viewport({}) {}

    HRESULT Init(ID3D11Device* device, UINT width, UINT height) {
        this->viewport = D3D11_VIEWPORT{
            .TopLeftX = 0,
            .TopLeftY = 0,
            .Width    = static_cast<FLOAT>(width),
            .Height   = static_cast<FLOAT>(height),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f,
        };

        this->position.Init(device, this->viewport.Width, this->viewport.Height);
        this->color.Init(device, this->viewport.Width, this->viewport.Height);
        this->normal.Init(device, this->viewport.Width, this->viewport.Height);

        D3D11_TEXTURE2D_DESC depthStencilDesc = {
            depthStencilDesc.Width              = width,
            depthStencilDesc.Height             = height,
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

    void BindLightingPass(ID3D11DeviceContext* context) {
        // Unbind Gbuffers from write
        ID3D11RenderTargetView* rVResetter[3] = {nullptr, nullptr, nullptr};
        context->OMSetRenderTargets(3, rVResetter, nullptr);

        // Bind to Compute
        ID3D11ShaderResourceView* srvs[3]{
            this->position.GetSRV(),
            this->color.GetSRV(),
            this->normal.GetSRV(),
        };
        context->CSSetShaderResources(0, 3, srvs);
    }
    void BindGeometryPass(ID3D11DeviceContext* context) {
        // Unbind Gbuffers from Compute
        ID3D11ShaderResourceView* rVResetter[3] = {nullptr, nullptr, nullptr};
        context->CSSetShaderResources(0, 3, rVResetter);

        // Bind Gbuffers for write
        ID3D11RenderTargetView* rendertargets[3] = {
            this->position.GetRTV(),
            this->color.GetRTV(),
            this->normal.GetRTV(),
        };
        context->OMSetRenderTargets(3, rendertargets, this->depthStencilView.Get());
    }
    void Clear(ID3D11DeviceContext* context, std::array<float, 4> clearColor) {
        float zero[4]{};
        context->ClearRenderTargetView(this->color.GetRTV(), clearColor.data());
        context->ClearRenderTargetView(this->position.GetRTV(), zero);
        context->ClearRenderTargetView(this->normal.GetRTV(), zero);

        context->ClearDepthStencilView(this->depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    }
    ID3D11DepthStencilView* GetDepthStencilView() const { return this->depthStencilView.Get(); }
    D3D11_VIEWPORT GetViewPort() const { return this->viewport; }

  private:
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencil;

    D3D11_VIEWPORT viewport;
    GBuffer position;
    GBuffer color;
    GBuffer normal;
};

class Camera {
  public:
    inline Camera(float horizontalFOVDegrees, float aspectRatio, float nearZ, float farZ, DirectX::XMVECTOR position,
                  DirectX::XMVECTOR quaternion, ID3D11UnorderedAccessView* UAV,
                  RenderingResources* rr);
    inline ~Camera() = default;

    Transform transform;

    inline DirectX::XMMATRIX createViewMatrix() const;
    inline DirectX::XMMATRIX createProjectionMatrix() const;

    inline float getAspectRatio() const;
    inline float getVerticalFOVRadians() const;
    inline float getNearZ() const;
    inline float getFarZ() const;
    inline RenderingResources* GetRenderResources() const;
    inline D3D11_VIEWPORT GetViewPort() const;

    inline void Update(InputHandler& input);

    inline ID3D11UnorderedAccessView** GetAdressOfUAV();

    inline ID3D11RenderTargetView* GetRTV() const;
    inline ID3D11RenderTargetView** GetAdressOfRTV();

  private:
    RenderingResources* rr;
    ID3D11UnorderedAccessView* UAV;

    float verticalFOVRadians;
    float aspectRatio;
    float nearZ;
    float farZ;
    float xRotation;
    float yRotation;
};

inline Camera::Camera(float horizontalFOVDegrees, float aspectRatio, float nearZ, float farZ,
                      DirectX::XMVECTOR position, DirectX::XMVECTOR quaternion, ID3D11UnorderedAccessView* UAV,
                      RenderingResources* rr)
    : aspectRatio(aspectRatio), nearZ(nearZ), farZ(farZ), transform(position, quaternion, {1, 1, 1}), xRotation(0.0f),
      yRotation(0.0f), UAV(UAV), rr(rr) {
    this->verticalFOVRadians = DirectX::XMConvertToRadians(horizontalFOVDegrees / aspectRatio);
}

inline DirectX::XMMATRIX Camera::createViewMatrix() const {
    DirectX::XMVECTOR forward = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 0, 1, 0),
                                                         this->transform.GetRotationQuaternion()); // Local forward
    DirectX::XMVECTOR up =
        DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 1, 0, 0), this->transform.GetRotationQuaternion()); // Local up
    return DirectX::XMMatrixLookToLH(this->transform.GetPosition(), forward, up);
}

inline DirectX::XMMATRIX Camera::createProjectionMatrix() const {
    return DirectX::XMMatrixPerspectiveFovLH(this->verticalFOVRadians, this->aspectRatio, this->nearZ, this->farZ);
}

inline float Camera::getAspectRatio() const { return this->aspectRatio; }

inline float Camera::getVerticalFOVRadians() const { return this->verticalFOVRadians; }

inline float Camera::getNearZ() const { return this->nearZ; }

inline float Camera::getFarZ() const { return this->farZ; }

inline RenderingResources* Camera::GetRenderResources() const { return this->rr; }

inline D3D11_VIEWPORT Camera::GetViewPort() const { return this->rr->GetViewPort(); }

inline ID3D11UnorderedAccessView** Camera::GetAdressOfUAV() { return &this->UAV; }

inline void Camera::Update(InputHandler& input) {

    // WASD movement
    const float speed = 0.069f;
    if (input.isDown('W')) {
        this->transform.Move(this->transform.GetDirectionVector(), speed);
    }
    if (input.isDown('A')) {
        this->transform.Move(DirectX::XMVector3Cross(this->transform.GetDirectionVector(), {0, 1, 0, 0}), speed);
    }
    if (input.isDown('S')) {
        this->transform.Move(DirectX::XMVectorNegate(this->transform.GetDirectionVector()), speed);
    }
    if (input.isDown('D')) {
        this->transform.Move(DirectX::XMVector3Cross(this->transform.GetDirectionVector(), {0, -1, 0, 0}), speed);
    }

    // Up and Down movement
    if (input.isDown(VK_SPACE)) {
        this->transform.Move({0, 1, 0, 0}, speed);
    }
    if (input.isDown(VK_CONTROL)) {
        this->transform.Move({0, -1, 0, 0}, speed);
    }

    // Hide cursor
    ShowCursor(FALSE);

    // Get screen center
    POINT screenCenter{
        .x = GetSystemMetrics(SM_CXSCREEN) / 2,
        .y = GetSystemMetrics(SM_CYSCREEN) / 2,
    };

    // Get current cursor
    POINT cursorPos;
    GetCursorPos(&cursorPos);

    // Calculate mouse movement
    int mouseDeltaX = cursorPos.x - screenCenter.x;
    int mouseDeltaY = cursorPos.y - screenCenter.y;

    // Reset cursor pos to center
    SetCursorPos(screenCenter.x, screenCenter.y);

    // Mouse movement
    const float sensitivity = 0.001f;
    const float clampMargin = 0.3f;
    float x                 = mouseDeltaX * sensitivity;
    float y                 = mouseDeltaY * sensitivity;

    // Rotate rotations
    this->xRotation += y;
    this->xRotation =
        std::clamp(this->xRotation, (-DirectX::XM_PIDIV2) + clampMargin, DirectX::XM_PIDIV2 - clampMargin);

    this->yRotation += x;

    // Apply rotations
    DirectX::XMVECTOR rotationQuat = DirectX::XMQuaternionRotationRollPitchYaw(this->xRotation, this->yRotation, 0);
    this->transform.SetRotationQuaternion(rotationQuat);
}

#endif