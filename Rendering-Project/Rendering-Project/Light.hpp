#ifndef LIGHT_HPP
#define LIGHT_HPP
#include "ConstantBuffer.hpp"
#include "Transform.hpp"

class Light {
  public:
    inline Light(Transform transform, DirectX::XMVECTOR color, float intensity, DirectX::XMVECTOR direction, float angle);
    ~Light() {};
    inline float GetIntesity() const { return this->intensity; }
    inline DirectX::XMVECTOR GetColor() const { return this->color; }
    Transform transform;
    DirectX::XMVECTOR GetDirection() const;
    float GetAngle() const; 

  private:
    DirectX::XMVECTOR color;
    DirectX::XMVECTOR direction;
    float angle;
    float intensity;
};

inline Light::Light(Transform transform, DirectX::XMVECTOR color, float intensity, DirectX::XMVECTOR direction,
                    float angle)
    : transform(transform), color(color), intensity(intensity), direction(direction), angle(DirectX::XMConvertToRadians(angle)) {}

inline DirectX::XMVECTOR Light::GetDirection() const { return this->direction; }

inline float Light::GetAngle() const { return this->angle; }

#endif
