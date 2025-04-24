#ifndef BASE_SCENE
#define BASE_SCENE

#include "Camera.hpp"
#include "DirectionalLight.hpp"
#include "Light.hpp"
#include "LightManager.hpp"
#include "ParticleSystem.hpp"
#include "SceneObject.hpp"
#include "SimpleObject.hpp"
#include "WindowHandler.hpp"
#include "QuadTree.hpp"
#include <d3d11_4.h>
#include "Mesh.hpp"

class BaseScene {
  public:
    BaseScene(Window& window);
    virtual ~BaseScene() {};

    HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* immediateContext);

    void AddSimpleObject(Transform transform, Mesh* mesh, bool dynamic, bool tesselate = true);
    void AddDCEM(Transform transform, ID3D11PixelShader* normalPS, ID3D11PixelShader* DCEMPS, Mesh* mesh, UINT size,
                 bool tesselate = true);

    void AddSpotLight(Transform transform, DirectX::XMVECTOR color, float angle, Mesh* lightMesh = nullptr);
    void AddDirLight(Transform transform, DirectX::XMVECTOR color, float width, float height, Mesh* lightMesh = nullptr);
    LightManager& GetLightManager();

    std::vector<Camera*>& GetCameras();
    std::vector<SceneObject*> GetBoundingBoxes();
    std::vector<SceneObject*> GetVisibleObjects(Camera& cam);
    std::vector<SceneObject*> GetObjects();

    ParticleSystem& GetParticleSystem();

    Camera& GetMainCam();

    virtual void UpdateScene(float deltaTime) = 0;

  protected:
    Mesh cubeMesh;

    LightManager lm;
    Camera mainCamera;
    InputHandler& input;
    ParticleSystem particleSystem;

    std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes;
    std::vector<std::unique_ptr<SceneObject>> objects;
    std::vector<std::unique_ptr<SceneObject>> staticObjects;
    std::vector<Camera*> cameras;

    QuadTree quadTree;

    std::vector<std::unique_ptr<SceneObject>> boundingBoxes;
    std::vector<SceneObject*> visibleObjects;
};
#endif