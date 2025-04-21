#ifndef SCENE_HPP
#define SCENE_HPP

#include "Camera.hpp"
#include "DirectionalLight.hpp"
#include "InputHandler.hpp"
#include "Light.hpp"
#include "LightManager.hpp"
#include "ParticleSystem.hpp"
#include "QuadTree.hpp"
#include "SceneObject.hpp"
#include "WindowHandler.hpp"
#include <memory>
#include <vector>

class Scene {
  public:
    Scene(Window& window);
    ~Scene();

    void AddSceneObject(SceneObject* sceneObject);
    void AddBoundingBox(SceneObject* box);

    void CreateObject(Mesh* mesh, const DirectX::XMVECTOR& position, ID3D11Device* device,
                      const std::string& folder = ".");

    void AddCameraObject(const Camera& camera);
    void AddLightObject(const Light& light);
    void AddDirLight(const DirectionalLight& light);
    LightManager& GetLightManager();

    HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* immediateContext);

    std::vector<Camera>& getCameras();
    const std::vector<Light>& getLights();
    std::vector<SceneObject*>& getObjects();
    std::vector<SceneObject*>& GetBoundingBoxes();
    std::vector<SceneObject*>& GetVisibleObjects();

    ParticleSystem& GetParticleSystem();

    Mesh* GetMesh(const std::string& folder, const std::string& objname, ID3D11Device* device);

    Camera& getMainCam();

    void UpdateScene();

  private:
    LightManager lm;
    Camera mainCamera;
    InputHandler& input;
    ParticleSystem particleSystem;

    std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes;
    std::vector<SceneObject*> objects;
    std::vector<Camera> cameras;
    std::vector<Light> lights;

    QuadTree quadTree;

    std::vector<SceneObject*> boundingBoxes;
    std::vector<SceneObject*> visibleObjects;
};

#endif