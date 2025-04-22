#include "BaseScene.hpp"
class TestScene : BaseScene {
  public:
    TestScene(Window& window, Renderer& renderer) : BaseScene(window) {
        Mesh* sphereMesh = renderer.GetMesh("./sphere", "icoSphere.obj");
        Mesh* boatMesh   = renderer.GetMesh("./boat", "boat.obj");
        Mesh* cubeMesh   = renderer.GetMesh("./NPCube2", "cube.obj");

        this->AddDCEM(Transform({10, 0, 0}), renderer.GetPS(), renderer.GetDCEMPS(), sphereMesh);
        this->AddDirLight(Transform({0, 0, 0}, 90 + 45, 0), {1, 1, 1}, 100, 100);
        this->AddSimpleObject(Transform({20, 0, 0}), boatMesh, false);

        this->Init(renderer.GetDevice(), renderer.GetDeviceContext());
    }
    void UpdateScene() override {}
};