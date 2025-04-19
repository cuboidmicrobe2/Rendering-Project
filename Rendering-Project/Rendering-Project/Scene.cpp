#include "Scene.hpp"
#include "SimpleObject.hpp"
#include <DirectXCollision.h>
#include <memory>

Scene::Scene(Window& window)
    : input(window.inputHandler), mainCamera(90, 16.f / 9.f, 1, 1000, {0, 0, -10}, {0, 0, 1}, nullptr, nullptr) {}

Scene::~Scene() {}

void Scene::AddSceneObject(SceneObject* sceneObject) {
    this->quadTree.AddElement(sceneObject, sceneObject->GetBoundingBox());
    this->objects.push_back(sceneObject);
}

void Scene::CreateObject(const std::string& objectFileName, const DirectX::XMVECTOR& position, ID3D11Device* device,
                         const std::string& folder) {

    // Object
    Mesh* mesh           = this->LoadMesh(folder, objectFileName, device);
    SimpleObject* object = new SimpleObject(Transform(position), mesh);
    object->InitBuffer(device);

    DirectX::BoundingBox boundingBox = object->GetBoundingBox();

    // Box
    Mesh* boxMesh = this->LoadMesh(folder, "cube.obj", device);
    SimpleObject* box =
        new SimpleObject(Transform(DirectX::XMLoadFloat3(&boundingBox.Center), DirectX::XMQuaternionIdentity(),
                                   DirectX::XMLoadFloat3(&boundingBox.Extents)),
                         boxMesh);
    box->InitBuffer(device);

    this->AddSceneObject(object);
    this->AddSceneObject(box);
}

void Scene::AddCameraObject(const Camera& camera) { this->cameras.emplace_back(camera); }

void Scene::AddLightObject(const Light& light) { this->lights.emplace_back(light); }

HRESULT Scene::Init(ID3D11Device* device, ID3D11DeviceContext* immediateContext) {
    // Load the particle shaders
    HRESULT result = this->particleSystem.LoadShaders(device, immediateContext);
    if (FAILED(result)) {
        std::cerr << "Particle system shaders failed\n";
        return result;
    }

    // Initialize particles with random values
    result =
        this->particleSystem.InitializeParticles(device, immediateContext, sizeof(Particle), 10000, false, true, true);
    if (FAILED(result)) {
        std::cerr << "Particle system particles failed\n";
        return result;
    }

    return S_OK;
}

std::vector<Camera>& Scene::getCameras() { return this->cameras; }

const std::vector<Light>& Scene::getLights() { return this->lights; }

std::vector<SceneObject*>& Scene::getObjects() { return this->objects; }

const std::vector<SceneObject*>& Scene::GetVisibleObjects() const { return this->visibleObjects; }

ParticleSystem& Scene::GetParticleSystem() { return this->particleSystem; }

Mesh* Scene::LoadMesh(const std::filesystem::path& folder, const std::string& objname, ID3D11Device* device) {
    this->meshes.emplace_back(new Mesh(device, folder, objname));
    return this->meshes.back().get();
}

Camera& Scene::getMainCam() { return this->mainCamera; }

void Scene::UpdateScene() {
    this->mainCamera.Update(this->input);

    // Create a frustum from the main camera's view and projection matrices
    DirectX::BoundingFrustum cameraFrustum;
    DirectX::XMMATRIX viewProj = DirectX::XMMatrixMultiplyTranspose(this->mainCamera.createViewMatrix(),
                                                                    this->mainCamera.createProjectionMatrix());
    DirectX::BoundingFrustum::CreateFromMatrix(cameraFrustum, viewProj);

    // Get visible objects using the quadtree
    std::vector<SceneObject*> visibleObjects = this->quadTree.CheckTree(cameraFrustum);

    // Update all objects
    for (SceneObject* obj : this->objects) {
        obj->Update();
    }

    // Store the list of visible objects for rendering
    this->visibleObjects.clear();
    for (const SceneObject* obj : visibleObjects) {
        this->visibleObjects.push_back(const_cast<SceneObject*>(obj));
    }
}