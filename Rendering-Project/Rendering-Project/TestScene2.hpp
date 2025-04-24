#ifndef TEST_SCENE2
#define TEST_SCENE2
#include "BaseScene.hpp"
#include "MeshHandler.hpp"

class TestScene2 : public BaseScene {
  public:
    TestScene2(Window& window, ID3D11Device* device, ID3D11DeviceContext* context, MeshHandler& meshHandler,
              ID3D11PixelShader* basePS, ID3D11PixelShader* DCEMPS)
        : BaseScene(window) {
        Mesh* sphereMesh = meshHandler.GetMesh("./sphere", "icoSphere.obj", device);
        Mesh* boatMesh   = meshHandler.GetMesh("./boat", "boat.obj", device);
        Mesh* cubeMesh   = meshHandler.GetMesh("./NPCube2", "cube.obj", device);

        this->AddDCEM(Transform({20, 3, 0}), basePS, DCEMPS, sphereMesh, 256);
        this->AddDirLight(Transform({0, 0, 0}, 90 + 45, 0), {1, 1, 1}, 100, 100);
        this->AddSimpleObject(Transform({20, 0, 0}), boatMesh, false, false);
        this->AddSimpleObject(Transform({-20, 0, 0}), boatMesh, false, false);
        this->AddSimpleObject(Transform({10, 0, 0}), cubeMesh, false, false);

        HRESULT hr = this->Init(device, context);
        if (FAILED(hr)) throw std::runtime_error("Failed to initialize scene!");
    }
    void UpdateScene(float deltaTime) override { this->mainCamera.Update(this->input, deltaTime); }
    ~TestScene2() {}
};

#endif