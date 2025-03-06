#ifndef LIGHT_HPP
#define LIGHT_HPP
#include "Transform.hpp"
#include "ConstantBuffer.hpp"

class Light {
  public:
    inline Light(Transform transform, DirectX::XMVECTOR color, float intensity);
    ~Light() {};

    Transform transform;

  private:
    DirectX::XMVECTOR color;
    float intensity;
};

inline Light::Light(Transform transform, DirectX::XMVECTOR color, float intensity)
    : transform(transform), color(color), intensity(intensity) {
}

#endif
