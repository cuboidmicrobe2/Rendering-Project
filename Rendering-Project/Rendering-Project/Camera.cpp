#include "Camera.hpp"

Camera::Camera(float horizontalFOVDegrees, float aspectRatio, float nearZ, float farZ, DirectX::XMVECTOR position,
               DirectX::XMVECTOR direction)
    : aspectRatio(aspectRatio), nearZ(nearZ), farZ(farZ), position(position) {
    this->position = DirectX::XMVector3Normalize(direction);
    this->verticalFOVRadians = DirectX::XMConvertToRadians(horizontalFOVDegrees / aspectRatio);
}

DirectX::XMMATRIX Camera::createViewMatrix() const {
    return DirectX::XMMatrixLookAtLH(this->position, DirectX::XMVectorAdd(this->position, this->direction),
                                     {0, 1, 0, 0});
}

DirectX::XMMATRIX Camera::createProjectionMatrix() const {
    return DirectX::XMMatrixPerspectiveFovLH(this->verticalFOVRadians, this->aspectRatio, this->nearZ, this->farZ);
}

inline void Camera::setDirection(float rotationX, float rotationY, float rotationZ) {
    this->direction = DirectX::XMQuaternionRotationRollPitchYaw(rotationX, rotationY, rotationZ);
}

void Camera::rotateCamera(float rotationX, float rotationY, float rotationZ) {
    DirectX::XMVECTOR quaternion = DirectX::XMQuaternionRotationRollPitchYaw(rotationX, rotationY, rotationZ);
    this->direction              = DirectX::XMVector3Rotate(this->direction, quaternion);
}

inline DirectX::XMVECTOR Camera::getPosition() const { return this->position; }

inline DirectX::XMVECTOR Camera::getDirection() const { return this->direction; }

inline float Camera::getAspectRatio() const { return this->aspectRatio; }

inline float Camera::getVerticalFOVRadians() const { return this->verticalFOVRadians; }

inline float Camera::getNearZ() const { return this->nearZ; }

inline float Camera::getFarZ() const { return this->farZ; }
