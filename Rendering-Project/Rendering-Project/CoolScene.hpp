#ifndef COOL_SCENE_HPP
#define COOL_SCENE_HPP
#include "BaseScene.hpp"
#include "MeshHandler.hpp"

class CoolScene : public BaseScene {
  public:
    CoolScene(Window& window, ID3D11Device* device, ID3D11DeviceContext* context, MeshHandler& meshHandler,
              ID3D11PixelShader* basePS, ID3D11PixelShader* DCEMPS)
        : BaseScene(window) {
        Mesh* cubeMesh    = meshHandler.GetMesh("./CoolScene", "cube.obj", device);
        Mesh* volcanoMesh = meshHandler.GetMesh("./CoolScene", "1230 Volcano.obj", device);
        Mesh* groundMesh  = meshHandler.GetMesh("./CoolScene", "Ground.obj", device);
        Mesh* chestMesh   = meshHandler.GetMesh("./CoolScene", "Chest Closed.obj", device);
        Mesh* boatMesh    = meshHandler.GetMesh("./boat", "boat.obj", device);
        Mesh* treeMesh    = meshHandler.GetMesh("./CoolScene", "Palm Tree.obj", device);
        Mesh* sphereMesh  = meshHandler.GetMesh("./sphere", "icoSphere.obj", device);
        Mesh* sharkMesh   = meshHandler.GetMesh("./CoolScene", "shark.obj", device);

        Mesh* blueCube = meshHandler.GetMesh("./BlueCube", "cube.obj", device);

        // "Sailing" boat
        this->AddSimpleObject(Transform({-45, -5.5, 10}, 0, 30), boatMesh, true, false);

        // Ground
        this->AddSimpleObject(Transform({1.7, -6.5, 2}, DirectX::XMQuaternionIdentity(), {2, 2, 2}), groundMesh, true,
                              false);

        // Palm Trees
        this->AddSimpleObject(Transform({2.4, -4.3, -11}, 0, 32), treeMesh, false, false);
        this->AddSimpleObject(Transform({2.4, -4.3, 20}, 0, 56), treeMesh, false, false);
        this->AddSimpleObject(Transform({20.4, -4.3, -11}, 0, 323), treeMesh, false, false);
        this->AddSimpleObject(Transform({22.4, -4.3, -9}, 0, 34), treeMesh, false, false);
        this->AddSimpleObject(Transform({20.4, -4.3, -8}, 0, 180), treeMesh, false, false);
        this->AddSimpleObject(Transform({20.4, -4.3, 11}, 0, 76), treeMesh, false, false);
        this->AddSimpleObject(Transform({22.4, -4.3, 9}, 0, 180), treeMesh, false, false);
        this->AddSimpleObject(Transform({20.4, -4.3, 8}, 0, 43), treeMesh, false, false);
        this->AddSimpleObject(Transform({10.4, -4.3, 11}, 0, 76), treeMesh, false, false);
        this->AddSimpleObject(Transform({12.4, -4.3, 9}, 0, 12), treeMesh, false, false);
        this->AddSimpleObject(Transform({10.4, -4.3, 8}, 0, 54), treeMesh, false, false);
        this->AddSimpleObject(Transform({-10, -4.3, -11}, 0, 98), treeMesh, false, false);
        this->AddSimpleObject(Transform({-12, -4.3, -9}, 0, 76), treeMesh, false, false);
        this->AddSimpleObject(Transform({-10, -4.3, -8}, 0, 76), treeMesh, false, false);

        // Ocean
        this->AddSimpleObject(Transform({0, -7, 0}, DirectX::XMQuaternionIdentity(), {1000, 1, 1000}), blueCube, true,
                              false);

        // Volcano
        this->AddSimpleObject(Transform({1.7, -4, 2}, DirectX::XMQuaternionIdentity(), {0.1, 0.1, 0.1}), volcanoMesh,
                              true, true, true);

        // Floating boxes
        this->AddSimpleObject(Transform({5, -5.5, 40}, 0, 30), cubeMesh, false, false);
        this->AddSimpleObject(Transform({37, -5.5, 10}, 0, 10), cubeMesh, false, false);
        this->AddSimpleObject(Transform({40, -5.5, 0}, 0, 78), cubeMesh, false, false);
        this->AddSimpleObject(Transform({35, -5.5, 30}, 0, 54), cubeMesh, false, false);
        this->AddSimpleObject(Transform({17, -5.5, -30}, 0, 234), cubeMesh, false, false);
        this->AddSimpleObject(Transform({-37, -5.5, -10}, 0, 56), cubeMesh, false, false);
        this->AddSimpleObject(Transform({-40, -5.5, 0}, 0, 30), cubeMesh, false, false);
        this->AddSimpleObject(Transform({-35, -5.5, 20}, 0, 85), cubeMesh, false, false);

        // Shark
        this->AddSimpleObject(Transform({-20, -6.9, 25}, -5, 85), sharkMesh, false, false);

        // Treasure Chest
        this->AddSimpleObject(Transform({5, -4.3, -10}, 0, 180), chestMesh, false, true);

        // Sailing boats
        this->AddSimpleObject(Transform({35, -5.5, 45}, 0, 330), boatMesh, false, false, false);
        this->AddSimpleObject(Transform({40, -5.5, 0}), boatMesh, false, false);

        // Sun
        Mesh* sphere = meshHandler.GetMesh("./sphere", "icoSphere.obj", device);
        this->AddDirLight(Transform({1.7, -4, 2}, 90 + 45, 0), {1, 1, 1}, 100, 100, sphere);

        HRESULT hr = this->Init(device, context);
        if (FAILED(hr)) throw std::runtime_error("Failed to initialize scene!");
    }
    void UpdateScene(float deltaTime) override {
        this->mainCamera.Update(this->input, deltaTime);
        DirectX::XMVECTOR center = {0, -5.5, 0};
        float radius             = 50;
        float angularSpeed       = 0.5;
        float rotation           = deltaTime * angularSpeed;
        SceneObject* obj         = this->dynamicObjects[0].get();
        obj->transform.Rotate(0, rotation);
        DirectX::XMVECTOR pos =
            DirectX::XMVectorScale(DirectX::XMVector3Cross({0, 1, 0}, obj->transform.GetDirectionVector()), radius);
        pos = DirectX::XMVectorAdd(pos, center);
        obj->transform.SetPosition(pos);
    }
};

#endif