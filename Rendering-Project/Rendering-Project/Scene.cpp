#include "Scene.hpp"

Scene::Scene(Window& window) : renderer(window) {}

Scene::~Scene() {}

HRESULT Scene::Init() { return this->renderer.Init(); }

void Scene::addSceneObject(const SceneObject& sceneObject) { this->objects.emplace_back(sceneObject); }

void Scene::addCameraObject(const Camera& camera) { this->cameras.emplace_back(camera); }

void Scene::renderScene() const {

}
