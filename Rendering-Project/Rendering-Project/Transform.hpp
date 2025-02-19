#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <DirectXMath.h>

class Transform {
  public:
    inline Transform();
    inline Transform(DirectX::XMVECTOR position, DirectX::XMVECTOR quaternion = DirectX::XMQuaternionIdentity(),
                     DirectX::XMVECTOR scale = {1, 1, 1});

    inline void SetPosition(DirectX::XMVECTOR position);
    inline void SetRotationRPY(float roll, float pitch, float yaw);
    inline void SetRotationRPY(DirectX::XMVECTOR rollPitchYaw);
    inline void SetRotationQuaternion(DirectX::XMVECTOR quaternion);

    inline void SetScale(DirectX::XMVECTOR scale);

    inline void Move(DirectX::XMVECTOR move);
    inline void Rotate(float rotationX, float rotationY, float rotationZ = 0);
    inline void RotateQuaternion(DirectX::XMVECTOR quaternion);

    inline DirectX::XMVECTOR GetPosition() const;
    inline DirectX::XMVECTOR GetRotationQuaternion() const;
    inline DirectX::XMVECTOR GetDirectionVector() const;
    inline DirectX::XMVECTOR GetScale() const;

  private:
    DirectX::XMVECTOR position;
    DirectX::XMVECTOR quaternion;
    DirectX::XMVECTOR scale;
};


inline Transform::Transform() : position({}), quaternion(DirectX::XMQuaternionIdentity()), scale({1, 1, 1}) {}

inline Transform::Transform(DirectX::XMVECTOR position, DirectX::XMVECTOR quaternion, DirectX::XMVECTOR scale)
    : position(position), quaternion(DirectX::XMQuaternionNormalize(quaternion)), scale(scale) {}

inline void Transform::SetPosition(DirectX::XMVECTOR position) { this->position = position; }

inline void Transform::SetRotationRPY(float roll, float pitch, float yaw) {
    this->quaternion = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
}

inline void Transform::SetRotationRPY(DirectX::XMVECTOR rollPitchYaw) {
    this->quaternion = DirectX::XMQuaternionRotationRollPitchYawFromVector(rollPitchYaw);
}

inline void Transform::SetRotationQuaternion(DirectX::XMVECTOR quaternion) {
    this->quaternion = DirectX::XMQuaternionNormalize(quaternion);
}

inline void Transform::SetScale(DirectX::XMVECTOR scale) { this->scale = scale; }

inline void Transform::Move(DirectX::XMVECTOR move) { this->position = DirectX::XMVectorAdd(this->position, move); }

inline void Transform::Rotate(float x, float y, float z) {
    this->RotateQuaternion(DirectX::XMQuaternionRotationRollPitchYaw(y, z, x));
}

inline void Transform::RotateQuaternion(DirectX::XMVECTOR quaternion) {
    this->quaternion = DirectX::XMQuaternionMultiply(this->quaternion, quaternion);
}

inline DirectX::XMVECTOR Transform::GetPosition() const { return this->position; }

inline DirectX::XMVECTOR Transform::GetRotationQuaternion() const { return this->quaternion; }

inline DirectX::XMVECTOR Transform::GetDirectionVector() const {
    return DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 0, 1, 0), this->quaternion);
}

inline DirectX::XMVECTOR Transform::GetScale() const { return this->scale; }

#endif
