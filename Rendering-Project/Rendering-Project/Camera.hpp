#ifndef CAMERA_HPP
#define CAMERA_HPP
#include "directxmath.h"
#include <array>
#include <d3d11.h>

class Camera {
  public:
    Camera(float horizontalFOVDegrees, float aspectRatio, float nearZ, float farZ, DirectX::XMVECTOR position,
           DirectX::XMVECTOR direction);
    ~Camera() = default;

    DirectX::XMMATRIX createViewMatrix() const;
    DirectX::XMMATRIX createProjectionMatrix() const;

    inline void rotateCamera(float rotationX, float rotationY, float rotationZ = 0);

    inline DirectX::XMVECTOR getPosition() const;
    inline DirectX::XMVECTOR getDirection() const;
    inline float getAspectRatio() const;
    inline float getVerticalFOVRadians() const;
    inline float getNearZ() const;
    inline float getFarZ() const;

    inline void setPosition(DirectX::XMVECTOR position) { this->position = position; };
    inline void setDirection(float rotationX, float rotationY, float rotationZ);

  private:
    float verticalFOVRadians;
    float aspectRatio;
    float nearZ;
    float farZ;
    DirectX::XMVECTOR position;
    DirectX::XMVECTOR direction;
};

#endif