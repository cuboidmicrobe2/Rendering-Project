#ifndef SCENE_HPP
#define SCENE_HPP

#include <vector>
#include "SceneObject.hpp"
#include "Camera.hpp"
#include "Renderer.hpp"


class Scene {
  public:
    Scene(Window& window);
    ~Scene();
    HRESULT Init();

    void addSceneObject(const SceneObject& sceneObject);
    void addCameraObject(const Camera& camera);
    void renderScene() const;
    // Add light to scene

  private:
    Renderer renderer;

    std::vector<SceneObject> objects;

    std::vector<Camera> cameras;

    // Cameras

    // 
};

#endif