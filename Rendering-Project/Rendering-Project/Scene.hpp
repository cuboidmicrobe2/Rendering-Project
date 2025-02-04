#ifndef SCENE_HPP
#define SCENE_HPP

#include <vector>
#include "SceneObject.hpp"
#include "Camera.hpp"


class Scene {
  public:
    Scene();
    ~Scene();

    void addSceneObject(const SceneObject& sceneObject);
    // Add camera to scene
    // Add light to scene

  private:
    std::vector<SceneObject> objects;

    std::vector<Camera> cameras;

    // Cameras

    // 
};

#endif