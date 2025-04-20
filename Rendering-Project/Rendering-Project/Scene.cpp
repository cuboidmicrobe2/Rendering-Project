#include "Scene.hpp"
#include "SimpleObject.hpp"
#include <DirectXCollision.h>
#include <memory>

Scene::Scene(Window& window)
    : input(window.inputHandler), mainCamera(90, 16.f / 9.f, 1, 1000, {0, 0, -10}, {0, 0, 1}, nullptr, nullptr),
      lm(256, 8192), quadTree({100.0f, 100.0f, 100.0f, 0.0f}, 6, 8) {}

Scene::~Scene() {}

void Scene::AddSceneObject(SceneObject* sceneObject) { this->objects.push_back(sceneObject); }

void Scene::AddBoundingBox(SceneObject* box) { this->boundingBoxes.push_back(box); }

void Scene::CreateObject(Mesh* mesh, const DirectX::XMVECTOR& position, ID3D11Device* device,
                         const std::string& folder) {

    // Object
    SimpleObject* object = new SimpleObject(Transform(position), mesh);
    object->InitBuffer(device);

    DirectX::BoundingBox boundingBox = object->GetBoundingBox();

    // Box
    Mesh* boxMesh = this->LoadMesh("./cube", "cube.obj", device);
    SimpleObject* box =
        new SimpleObject(Transform(DirectX::XMLoadFloat3(&boundingBox.Center), DirectX::XMQuaternionIdentity(),
                                   DirectX::XMLoadFloat3(&boundingBox.Extents)),
                         boxMesh);
    box->InitBuffer(device);

    this->AddSceneObject(object);
    this->AddBoundingBox(box);
    this->quadTree.AddElement(object);
    this->quadTree.AddElement(box);
}

void Scene::AddCameraObject(const Camera& camera) { this->cameras.emplace_back(camera); }

void Scene::AddLightObject(const Light& light) { lm.AddSpotLight(light); }

void Scene::AddDirLight(const DirectionalLight& light) { this->lm.AddDirectionalLight(light); }

LightManager& Scene::GetLightManager() { return this->lm; }

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

    result = this->lm.Init(device);
    if (FAILED(result)) {
        std::cerr << "LightManager Init failed\n";
        return result;
    }

    return S_OK;
}

std::vector<Camera>& Scene::getCameras() { return this->cameras; }

const std::vector<Light>& Scene::getLights() { return this->lm.GetSpotLights(); }

std::vector<SceneObject*>& Scene::getObjects() { return this->objects; }

std::vector<SceneObject*>& Scene::GetBoundingBoxes() { return this->boundingBoxes; }

std::vector<SceneObject*>& Scene::GetVisibleObjects() { return this->visibleObjects; }

ParticleSystem& Scene::GetParticleSystem() { return this->particleSystem; }

Mesh* Scene::LoadMesh(const std::string& folder, const std::string& objname, ID3D11Device* device) {
    this->meshes.emplace_back(new Mesh(device, folder, objname));
    return this->meshes.back().get();
}

Camera& Scene::getMainCam() { return this->mainCamera; }

void Scene::UpdateScene() {
    this->mainCamera.Update(this->input);

    Camera frustumFOV = this->mainCamera;
    frustumFOV.SetFOV(this->mainCamera.GetFOV() - 20.0f);
    // Create a frustum from the main camera's view and projection matrices
    DirectX::BoundingFrustum cameraFrustum;
    DirectX::XMMATRIX proj = frustumFOV.createProjectionMatrix();
    DirectX::BoundingFrustum::CreateFromMatrix(cameraFrustum, proj);

    DirectX::XMMATRIX view  = frustumFOV.createViewMatrix();
    DirectX::XMMATRIX world = DirectX::XMMatrixInverse(nullptr, view);
    cameraFrustum.Transform(cameraFrustum, world);

    // Get visible objects using the quadtree
    this->visibleObjects.clear();
    std::vector<SceneObject*> visibleObjects = this->quadTree.CheckTree(cameraFrustum);

    // Update all objects
    for (SceneObject* obj : this->objects) {
        obj->Update();
    }

    // Store the list of visible objects for rendering
    for (const SceneObject* obj : visibleObjects) {
        this->visibleObjects.push_back(const_cast<SceneObject*>(obj));
    }
}