#ifndef SCENE_HPP
#define SCENE_HPP

#include "Camera.hpp"
#include "Light.hpp"
#include "Renderer.hpp"
#include "SceneObject.hpp"
#include <vector>

class Scene {
  public:
    Scene(Window& window);
    ~Scene();
    HRESULT Init();

    void AddSceneObject(const SceneObject& sceneObject);
    void AddCameraObject(const Camera& camera);
    void AddLightObject(const Light& light);

    void RenderScene(ID3D11Device* device, ID3D11DeviceContext* context) const;

    void UpdateScene();

  private:
    Renderer renderer;

    std::vector<SceneObject> objects;
    std::vector<Camera> cameras;
    std::vector<Light> lights;
};

#endif