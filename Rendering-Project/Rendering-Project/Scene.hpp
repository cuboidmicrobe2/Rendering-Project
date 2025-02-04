#ifndef SCENE_HPP
#define SCENE_HPP

#include <vector>
#include "SceneObject.hpp"


class Scene {
  public:
    Scene();
    ~Scene();

    void addSceneObject(const SceneObject& sceneObject);
    // Add camera to scene
    // Add light to scene

  private:
    std::vector<SceneObject> objects;

    // Lights

    // Cameras

    // 
};

#endif