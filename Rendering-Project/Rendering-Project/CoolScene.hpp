#ifndef COOL_SCENE_HPP
#define COOL_SCENE_HPP
#include "BaseScene.hpp"
#include "MeshHandler.hpp"

class CoolScene : public BaseScene {
  public:
    CoolScene(Window& window, ID3D11Device* device, ID3D11DeviceContext* context, MeshHandler& meshHandler,
              ID3D11PixelShader* basePS, ID3D11PixelShader* DCEMPS)
        : BaseScene(window) {
        Mesh* sphereMesh = meshHandler.GetMesh("./sphere", "icoSphere.obj", device);
        Mesh* boatMesh   = meshHandler.GetMesh("./boat", "boat.obj", device);
        Mesh* cubeMesh   = meshHandler.GetMesh("./NPCube2", "cube.obj", device);

        // Ground
        this->AddSimpleObject(Transform({0, -20, 0}, DirectX::XMQuaternionIdentity(), {5, 5, 5}), cubeMesh, false);

        // Sun
        // this->AddDirLight(Transform({0, 0, 0}, 90 + 45, 0), {1, 1, 1}, 100, 100);
        this->AddSpotLight(Transform({0, -10, 0}), {1, 1, 1}, 270);

        HRESULT hr = this->Init(device, context);
        if (FAILED(hr)) throw std::runtime_error("Failed to initialize scene!");
    }
    void UpdateScene() override { this->mainCamera.Update(this->input); }
};

#endif