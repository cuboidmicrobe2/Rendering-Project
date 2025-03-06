#ifndef SCENE_HPP
#define SCENE_HPP

#include "Camera.hpp"
#include "Light.hpp"
#include "ForwardRenderer.hpp"
#include "SceneObject.hpp"
#include <vector>
#include "InputHandler.hpp"
#include <memory>

class Scene {
  public:
    Scene(Window& window);
    ~Scene();
    HRESULT Init();

    void AddSceneObject(const SceneObject& sceneObject);
    void AddCameraObject(const Camera& camera);
    void AddLightObject(const Light& light);

    Mesh* LoadMesh(std::string path);

    void RenderScene();

    void UpdateScene();

  private:
    DaddyRenderer* renderer;
    InputHandler& inputhandler;

    std::vector<std::unique_ptr<Mesh>> meshes;
    std::vector<SceneObject> objects;
    std::vector<Camera> cameras;
    std::vector<Light> lights;
};

#endif