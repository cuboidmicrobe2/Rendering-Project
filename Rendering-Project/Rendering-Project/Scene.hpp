#ifndef SCENE_HPP
#define SCENE_HPP

#include "Camera.hpp"
#include "InputHandler.hpp"
#include "WindowHandler.hpp"
#include "Light.hpp"
#include "SceneObject.hpp"
#include <memory>
#include <vector>

class Scene {
  public:
    Scene(Window& window);
    ~Scene();

    void AddSceneObject(const SceneObject& sceneObject);
    void AddCameraObject(const Camera& camera);
    void AddLightObject(const Light& light);

    std::vector<Camera>& getCameras();
    const std::vector<Light>& getLights();
    const std::vector<SceneObject>& getObjects();

    Mesh* LoadMesh(const std::filesystem::path& folder, const std::string& objname, ID3D11Device* device);

    Camera& getMainCam();

    void UpdateScene();

  private:
    Camera mainCamera;
    InputHandler& input;

    std::vector<std::unique_ptr<Mesh>> meshes;
    std::vector<SceneObject> objects;
    std::vector<Camera> cameras;
    std::vector<Light> lights;
};

#endif