#ifndef BASE_SCENE_HPP
#define BASE_SCENE_HPP

#include "Camera.hpp"
#include "DirectionalLight.hpp"
#include "Light.hpp"
#include "LightManager.hpp"
#include "Mesh.hpp"
#include "ParticleSystem.hpp"
#include "QuadTree.hpp"
#include "SceneObject.hpp"
#include "SimpleObject.hpp"
#include "WindowHandler.hpp"
#include <d3d11_4.h>

namespace RenderingModes {
enum RenderingMode {
    Standard, Diffuse, Normals, Ambient, Specular, Position
};
}

class BaseScene {
  public:
    BaseScene(Window& window);
    virtual ~BaseScene() {};

    HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* immediateContext);

    void AddSimpleObject(Transform transform, Mesh* mesh, bool dynamic, bool tesselate = true,
                         bool showTessellation = false);
    void AddDCEM(Transform transform, ID3D11PixelShader* normalPS, ID3D11PixelShader* DCEMPS, Mesh* mesh, UINT size,
                 bool tesselate = true, bool showTessellation = false);

    void AddSpotLight(Transform transform, DirectX::XMVECTOR color, float angle, Mesh* lightMesh = nullptr);
    void AddDirLight(Transform transform, DirectX::XMVECTOR color, float width, float height,
                     Mesh* lightMesh = nullptr);
    LightManager& GetLightManager();

    std::vector<Camera*>& GetCameras();
    std::vector<SceneObject*> GetBoundingBoxes();
    std::vector<SceneObject*> GetVisibleObjects(Camera& cam);
    std::vector<SceneObject*> GetObjects();

    ParticleSystem& GetParticleSystem();

    Camera& GetMainCam();

    virtual void UpdateScene(float deltaTime) = 0;

    RenderingModes::RenderingMode GetRenderingMode() const { return this->renderingMode; }
    void SetRenderingMode(RenderingModes::RenderingMode mode) { this->renderingMode = mode; }

  protected:
    RenderingModes::RenderingMode renderingMode = RenderingModes::Standard;
    Mesh cubeMesh;

    LightManager lm;
    Camera mainCamera;
    InputHandler& input;
    ParticleSystem particleSystem;

    std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes;
    std::vector<std::unique_ptr<SceneObject>> objects;
    std::vector<std::unique_ptr<SceneObject>> dynamicObjects;
    std::vector<Camera*> cameras;

    QuadTree quadTree;

    std::vector<std::unique_ptr<SceneObject>> boundingBoxes;
    std::vector<SceneObject*> visibleObjects;
};
#endif