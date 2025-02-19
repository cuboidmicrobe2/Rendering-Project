#ifndef CAMERA_HPP
#define CAMERA_HPP
#include "Transform.hpp"
#include "directxmath.h"
#include <array>
#include <d3d11.h>

class Camera {
  public:
    inline Camera(float horizontalFOVDegrees, float aspectRatio, float nearZ, float farZ, DirectX::XMVECTOR position,
                  DirectX::XMVECTOR quaternion);
    inline ~Camera() = default;

    Transform transform;

    inline DirectX::XMMATRIX createViewMatrix() const;
    inline DirectX::XMMATRIX createProjectionMatrix() const;

    inline float getAspectRatio() const;
    inline float getVerticalFOVRadians() const;
    inline float getNearZ() const;
    inline float getFarZ() const;

  private:
    float verticalFOVRadians;
    float aspectRatio;
    float nearZ;
    float farZ;
};

inline Camera::Camera(float horizontalFOVDegrees, float aspectRatio, float nearZ, float farZ,
                      DirectX::XMVECTOR position, DirectX::XMVECTOR quaternion)
    : aspectRatio(aspectRatio), nearZ(nearZ), farZ(farZ), transform(position, quaternion, {1, 1, 1}) {
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

#endif