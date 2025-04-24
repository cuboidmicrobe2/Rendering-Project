#ifndef TEST_SCENE_HPP
#define TEST_SCENE_HPP
#include "BaseScene.hpp"
#include "MeshHandler.hpp"

class TestScene : public BaseScene {
  public:
    TestScene(Window& window, ID3D11Device* device, ID3D11DeviceContext* context, MeshHandler& meshHandler,
              ID3D11PixelShader* basePS, ID3D11PixelShader* DCEMPS)
        : BaseScene(window) {
        Mesh* sphereMesh = meshHandler.GetMesh("./sphere", "icoSphere.obj", device);
        Mesh* cubeMesh   = meshHandler.GetMesh("./NPCube2", "cube.obj", device);
        Mesh* floorMesh   = meshHandler.GetMesh("./CoolScene", "cube.obj", device);

        this->AddSimpleObject(Transform({0, 1, 0}), cubeMesh, false, false);
        this->AddSimpleObject(Transform({8, 1, 8}), cubeMesh, false, false);
        this->AddSimpleObject(Transform({3, 1, 0}, 0, 20), cubeMesh, false, false);
        this->AddSimpleObject(Transform({0, 1, 3}, 0, 45), cubeMesh, false, false);
        this->AddSimpleObject(Transform({-8, 1, 8}, 0, 20), sphereMesh, false, true, true);

        this->AddDCEM(Transform({-2, 1, -2}, 0, 20), basePS, DCEMPS, sphereMesh, 256, true);

        // floor
        this->AddSimpleObject(Transform({0, -1, 0}, DirectX::XMQuaternionIdentity(), {50, 1, 50}), floorMesh, true,
                              false);

        this->AddDirLight(Transform({0, 0, 0}, 135, 0), {0.5, 0.5, 0.5, 1}, 100, 100, sphereMesh);

        this->AddSpotLight(Transform({0, 8, -8}, 45, 0, 0, {.1, .1, .1}), {0, 0, 20, 1}, 80, sphereMesh);
        this->AddSpotLight(Transform({-8, 8, 0}, Transform::GetCameraRotationQuaternion(90, -45), {.1, .1, .1}),
                           {20, 0, 0, 1}, 80, sphereMesh);

        this->AddSpotLight(Transform({8, 8, 0}, Transform::GetCameraRotationQuaternion(-90, 45), {.1, .1, .1}),
                           {0, 20, 0, 1}, 80, sphereMesh);

        HRESULT hr = this->Init(device, context);
        if (FAILED(hr)) throw std::runtime_error("Failed to initialize scene!");
    }
    void UpdateScene(float deltaTime) override { 
        this->mainCamera.Update(this->input, deltaTime);
        SceneObject* obj = this->objects[0].get();
        obj->transform.Rotate(0, deltaTime * 1);
    }
    ~TestScene() {}
};

#endif