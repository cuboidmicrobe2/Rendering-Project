#ifndef SCENE_HPP
#define SCENE_HPP

#include "Camera.hpp"
#include "DeferredRenderer.hpp"
#include "ForwardRenderer.hpp"
#include "InputHandler.hpp"
#include "Light.hpp"
#include "SceneObject.hpp"
#include <memory>
#include <vector>

class Scene {
  public:
    Scene(Window& window);
    ~Scene();
    HRESULT Init();

    void AddSceneObject(const SceneObject& sceneObject);
    void AddCameraObject(const Camera& camera);
    void AddLightObject(const Light& light);

    Mesh* LoadMesh(const std::filesystem::path& folder, const std::string& objname);

    void RenderScene();

    void UpdateScene();

  private:
    std::unique_ptr<DaddyRenderer> renderer;
    InputHandler& input;

    std::vector<std::unique_ptr<Mesh>> meshes;
    std::vector<SceneObject> objects;
    std::vector<Camera> cameras;
    std::vector<Light> lights;
};

#endif