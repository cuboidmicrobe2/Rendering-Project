#ifndef CAMERA_HPP
#define CAMERA_HPP
#include "InputHandler.hpp"
#include "Transform.hpp"
#include "directxmath.h"
#include <algorithm>
#include <array>
#include <d3d11.h>
#include <wrl/client.h>

class Camera {
  public:
    inline Camera(float horizontalFOVDegrees, float aspectRatio, float nearZ, float farZ, DirectX::XMVECTOR position,
                  DirectX::XMVECTOR quaternion, ID3D11UnorderedAccessView* UAV, D3D11_VIEWPORT viewport = {});
    inline ~Camera() = default;

    Transform transform;

    inline DirectX::XMMATRIX createViewMatrix() const;
    inline DirectX::XMMATRIX createProjectionMatrix() const;

    inline float getAspectRatio() const;
    inline float getVerticalFOVRadians() const;
    inline float getNearZ() const;
    inline float getFarZ() const;
    inline D3D11_VIEWPORT& GetViewPort();

    inline void Update(InputHandler& input);

    inline ID3D11UnorderedAccessView** GetAdressOfUAV();


  private:
    D3D11_VIEWPORT viewport;
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
                      D3D11_VIEWPORT viewport)
    : aspectRatio(aspectRatio), nearZ(nearZ), farZ(farZ), transform(position, quaternion, {1, 1, 1}), xRotation(0.0f),
      yRotation(0.0f), UAV(UAV), viewport(viewport) {
    this->verticalFOVRadians = DirectX::XMConvertToRadians(horizontalFOVDegrees / aspectRatio);
}

inline DirectX::XMMATRIX Camera::createViewMatrix() const {
    return DirectX::XMMatrixLookToLH(this->transform.GetPosition(), this->transform.GetDirectionVector(), {0, 1, 0, 0});
}

inline DirectX::XMMATRIX Camera::createProjectionMatrix() const {
    return DirectX::XMMatrixPerspectiveFovLH(this->verticalFOVRadians, this->aspectRatio, this->nearZ, this->farZ);
}

inline float Camera::getAspectRatio() const { return this->aspectRatio; }

inline float Camera::getVerticalFOVRadians() const { return this->verticalFOVRadians; }

inline float Camera::getNearZ() const { return this->nearZ; }

inline float Camera::getFarZ() const { return this->farZ; }

inline D3D11_VIEWPORT& Camera::GetViewPort() { return this->viewport; }

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