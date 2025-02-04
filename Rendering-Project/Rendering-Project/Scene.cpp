#include "Scene.hpp"

Scene::Scene() {}

Scene::~Scene() {}

void Scene::addSceneObject(const SceneObject& sceneObject) { this->objects.emplace_back(sceneObject); }
