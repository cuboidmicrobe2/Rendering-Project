#ifndef LIGHT_HPP
#define LIGHT_HPP
#include "ConstantBuffer.hpp"
#include "Transform.hpp"

class Light {
  public:
    inline Light(Transform transform, DirectX::XMVECTOR color, float intensity);
    ~Light() {};
    inline float GetIntesity() const { return this->intensity; }
    inline DirectX::XMVECTOR GetColor() const { return this->color; }
    Transform transform;

  private:
    DirectX::XMVECTOR color;
    float intensity;
};

inline Light::Light(Transform transform, DirectX::XMVECTOR color, float intensity)
    : transform(transform), color(color), intensity(intensity) {}

#endif
