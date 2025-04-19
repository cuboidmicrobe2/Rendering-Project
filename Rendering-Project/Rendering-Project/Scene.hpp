#ifndef SCENE_HPP
#define SCENE_HPP

#include "Camera.hpp"
#include "InputHandler.hpp"
#include "Light.hpp"
#include "ParticleSystem.hpp"
#include "SceneObject.hpp"
#include "WindowHandler.hpp"
#include <memory>
#include <vector>
#include "LightManager.hpp"
#include "DirectionalLight.hpp"

class Scene {
  public:
    Scene(Window& window);
    ~Scene();

    void AddSceneObject(SceneObject* sceneObject);
    void AddCameraObject(const Camera& camera);
    void AddLightObject(const Light& light);
    void AddDirLight(const DirectionalLight& light);
    LightManager& GetLightManager();

    HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* immediateContext);

    std::vector<Camera>& getCameras();
    const std::vector<Light>& getLights();
    std::vector<SceneObject*>& getObjects();

    ParticleSystem& GetParticleSystem();

    Mesh* LoadMesh(const std::filesystem::path& folder, const std::string& objname, ID3D11Device* device);

    Camera& getMainCam();

    void UpdateScene();

  private:
    LightManager lm;
    Camera mainCamera;
    InputHandler& input;
    ParticleSystem particleSystem;

    std::vector<std::unique_ptr<Mesh>> meshes;
    std::vector<SceneObject*> objects;
    std::vector<Camera> cameras;
};

#endif