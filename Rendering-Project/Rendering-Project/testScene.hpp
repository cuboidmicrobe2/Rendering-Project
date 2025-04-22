#ifndef TEST_SCENE
#define TEST_SCENE

#include "BaseScene.hpp"
class TestScene : public BaseScene {
  public:
    TestScene(Window& window, ID3D11Device* device, ID3D11DeviceContext* context, MeshHandler& renderer,
              ID3D11PixelShader* basePS, ID3D11PixelShader* DCEMPS)
        : BaseScene(window) {
        Mesh* sphereMesh = renderer.GetMesh("./sphere", "icoSphere.obj", device);
        Mesh* boatMesh   = renderer.GetMesh("./boat", "boat.obj", device);
        Mesh* cubeMesh   = renderer.GetMesh("./NPCube2", "cube.obj", device);

        this->AddDCEM(Transform({20, 3, 0}), basePS, DCEMPS, sphereMesh, 256);
        this->AddDirLight(Transform({0, 0, 0}, 90 + 45, 0), {1, 1, 1}, 100, 100);
        this->AddSimpleObject(Transform({20, 0, 0}), boatMesh, false);

        HRESULT hr = this->Init(device, context);
        if (FAILED(hr)) throw std::runtime_error("Failed to initialize scene!");
    }
    void UpdateScene() override { 
        this->mainCamera.Update(this->input);
    }
};

#endif