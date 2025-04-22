#include "BaseScene.hpp"
#include "DCEM.hpp"

BaseScene::BaseScene(Window& window)
    : input(window.inputHandler), mainCamera(90, 16.f / 9.f, 1, 1000, {0, 0, -10}, {0, 0, 1}, nullptr, nullptr),
      lm(256, 8192), quadTree({100.0f, 100.0f, 100.0f, 0.0f}, 6, 8) {}

BaseScene::~BaseScene() {}

HRESULT BaseScene::Init(ID3D11Device* device, ID3D11DeviceContext* immediateContext) {
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

    this->cubeMesh.Initialize(device, "./cube", "cube.obj");

    for (const auto& obj : this->objects) {
        obj.get()->Init(device);
    }

    for (const auto& obj : this->boundingBoxes) {
        obj.get()->Init(device);
    }    

    return S_OK;
}

void BaseScene::AddSimpleObject(Transform transform, Mesh* mesh, bool dynamic) {
    SimpleObject* object             = new SimpleObject(transform, mesh);
    DirectX::BoundingBox boundingBox = object->GetBoundingBox();
    if (!dynamic) {
        SimpleObject* box =
            new SimpleObject(Transform(DirectX::XMLoadFloat3(&boundingBox.Center), DirectX::XMQuaternionIdentity(),
                                       DirectX::XMLoadFloat3(&boundingBox.Extents)),
                             &this->cubeMesh);
        this->objects.emplace_back(object);
        this->boundingBoxes.emplace_back(box);
        this->quadTree.AddElement(object);
        this->quadTree.AddElement(box);
    } else {
        this->staticObjects.emplace_back(object);
    }
}

void BaseScene::AddDCEM(Transform transform, ID3D11PixelShader* normalPS, ID3D11PixelShader* DCEMPS, Mesh* mesh, UINT size) {
    DCEM* object                     = new DCEM(transform, normalPS, DCEMPS, mesh, size);
    DirectX::BoundingBox boundingBox = object->GetBoundingBox();
    SimpleObject* box =
        new SimpleObject(Transform(DirectX::XMLoadFloat3(&boundingBox.Center), DirectX::XMQuaternionIdentity(),
                                   DirectX::XMLoadFloat3(&boundingBox.Extents)),
                         &this->cubeMesh);

    this->objects.emplace_back(object);
    this->boundingBoxes.emplace_back(box);
    this->quadTree.AddElement(object);
    this->quadTree.AddElement(box);

    for (Camera& cam : object->GetCameras())
        this->cameras.push_back(&cam);
}

void BaseScene::AddSpotLight(Transform transform, DirectX::XMVECTOR color, float angle) {
    Light l(transform, color, 0, angle);
    this->lm.AddSpotLight(l);
}

void BaseScene::AddDirLight(Transform transform, DirectX::XMVECTOR color, float width, float height) {
    DirectionalLight l(transform, color, 0, width, height);
    this->lm.AddDirectionalLight(l);
}

LightManager& BaseScene::GetLightManager() { return this->lm; }

std::vector<Camera*>& BaseScene::GetCameras() { return this->cameras; }

std::vector<SceneObject*> BaseScene::GetBoundingBoxes() {
    std::vector<SceneObject*> boxes;
    boxes.reserve(this->boundingBoxes.size());
    for (const auto& box : this->boundingBoxes) {
        boxes.emplace_back(box.get());
    }
    return boxes;
}

std::vector<SceneObject*> BaseScene::GetVisibleObjects(Camera& cam) { 
    Camera frustumFOV = cam;
    frustumFOV.SetFOV(this->mainCamera.GetFOV() - 20.0f);
    // Create a frustum from the main camera's view and projection matrices
    DirectX::BoundingFrustum cameraFrustum;
    DirectX::XMMATRIX proj = frustumFOV.createProjectionMatrix();
    DirectX::BoundingFrustum::CreateFromMatrix(cameraFrustum, proj);

    DirectX::XMMATRIX view  = frustumFOV.createViewMatrix();
    DirectX::XMMATRIX world = DirectX::XMMatrixInverse(nullptr, view);
    cameraFrustum.Transform(cameraFrustum, world);

    // Get visible objects using the quadtree
    std::vector<SceneObject*> objects = this->quadTree.CheckTree(cameraFrustum);

    for (const auto& obj : this->staticObjects) {
        objects.emplace_back(obj.get());
    }
    return objects; 
}

std::vector<SceneObject*> BaseScene::GetObjects() { 
    std::vector<SceneObject*> objects;
    objects.reserve(this->objects.size() + this->staticObjects.size());
    for (const auto& obj : this->objects) {
        objects.emplace_back(obj.get());
    }
    for (const auto& obj : this->staticObjects) {
        objects.emplace_back(obj.get());
    }
    return objects;
}

ParticleSystem& BaseScene::GetParticleSystem() { return this->particleSystem; }

Camera& BaseScene::GetMainCam() { return this->mainCamera; }
