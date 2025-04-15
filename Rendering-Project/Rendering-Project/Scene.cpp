#include "Scene.hpp"

Scene::Scene(Window& window)
    : input(window.inputHandler), mainCamera(90, 16.f / 9.f, 1, 1000, {0, 0, -10}, {0, 0, 1}, nullptr) {}

Scene::~Scene() {}

void Scene::AddSceneObject(const SceneObject& sceneObject) { this->objects.emplace_back(sceneObject); }

void Scene::AddCameraObject(const Camera& camera) { this->cameras.emplace_back(camera); }

void Scene::AddLightObject(const Light& light) { this->lights.emplace_back(light); }

HRESULT Scene::Init(ID3D11Device* device) {
    HRESULT result = this->particleSystem.Initialize(device, 64, 1000, true, true, true);
    if (FAILED(result)) {
        return result;
    }

    return this->particleSystem.LoadShaders();
}

std::vector<Camera>& Scene::getCameras() { return this->cameras; }

const std::vector<Light>& Scene::getLights() { return this->lights; }

const std::vector<SceneObject>& Scene::getObjects() { return this->objects; }

ParticleSystem& Scene::GetParticleSystem() { return this->particleSystem; }

Mesh* Scene::LoadMesh(const std::filesystem::path& folder, const std::string& objname, ID3D11Device* device) {
    this->meshes.emplace_back(new Mesh(device, folder, objname));
    return this->meshes.back().get();
}

Camera& Scene::getMainCam() { return this->mainCamera; }

void Scene::UpdateScene() {
    this->mainCamera.Update(this->input);

    for (SceneObject& obj : this->objects) {
        obj.Update();
    }
}