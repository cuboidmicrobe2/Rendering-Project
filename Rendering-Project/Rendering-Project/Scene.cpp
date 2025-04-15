#include "Scene.hpp"

Scene::Scene(Window& window) : input(window.inputHandler) {
    this->renderer = std::make_unique<DeferredRenderer>(window);
    if (FAILED(this->renderer->Init())) throw std::runtime_error("Failed to init renderer");
}

Scene::~Scene() {}

HRESULT Scene::Init() { return this->renderer->Init(); }

void Scene::AddSceneObject(const SceneObject& sceneObject) { this->objects.emplace_back(sceneObject); }

void Scene::AddCameraObject(const Camera& camera) { this->cameras.emplace_back(camera); }

void Scene::AddLightObject(const Light& light) { this->lights.emplace_back(light); }

Mesh* Scene::LoadMesh(const std::filesystem::path& folder, const std::string& objname) {
    this->meshes.emplace_back(new Mesh(this->renderer->GetDevice(), folder, objname));
    return this->meshes.back().get();
}

void Scene::RenderScene() {
    // Disable culling

    // Bind Lights
     ConstantBuffer lights;
     int32_t nrOfLights = this->lights.size();
     struct LightData {
         float pos[3];
         float intensity;
         float color[4];
     };
     std::vector<LightData> lightData;
     for (auto light : this->lights) {
         float* tempPos = light.transform.GetPosition().m128_f32;
         float* tempColor = light.GetColor().m128_f32;

         LightData l{
             .pos{tempPos[0], tempPos[1], tempPos[2]},
             .intensity = light.GetIntesity(),
             .color     = {tempColor[0], tempColor[1], tempColor[2], tempColor[3]},
         };
         lightData.emplace_back(l);
     }
     struct CSMetadata {
         int nrofLights = nrofLights;
         float cameraPos[3];
     };

     float* t = this->cameras[0].transform.GetPosition().m128_f32;
     CSMetadata metaData{.nrofLights = nrOfLights, .cameraPos = {t[0], t[1], t[2]}};

     lights.Initialize(this->renderer->GetDevice(), lightData.size() * sizeof(LightData), lightData.data());
     ConstantBuffer CSMetaData;
     CSMetaData.Initialize(this->renderer->GetDevice(), sizeof(metaData), &metaData);
     this->renderer->GetContext()->CSSetConstantBuffers(0, 1, CSMetaData.GetAdressOfBuffer());
     this->renderer->GetContext()->CSSetConstantBuffers(1, 1, lights.GetAdressOfBuffer());

    // Create and Bind view and projection matrixes
    ConstantBuffer viewAndProjectionMatrices;
    DirectX::XMFLOAT4X4 matrices[3];
    DirectX::XMMATRIX viewMatrix       = this->cameras.front().createViewMatrix();
    DirectX::XMMATRIX projectionMatrix = this->cameras.front().createProjectionMatrix();
    DirectX::XMStoreFloat4x4(&matrices[0], viewMatrix);
    DirectX::XMStoreFloat4x4(&matrices[1], projectionMatrix);
    DirectX::XMStoreFloat4x4(&matrices[2], DirectX::XMMatrixMultiplyTranspose(viewMatrix, projectionMatrix));
    viewAndProjectionMatrices.Initialize(this->renderer->GetDevice(), sizeof(matrices), matrices);
    this->renderer->GetContext()->VSSetConstantBuffers(0, 1, viewAndProjectionMatrices.GetAdressOfBuffer());

    for (const SceneObject& obj : this->objects) {
        obj.Draw(this->renderer->GetDevice(), this->renderer->GetContext());
    }

    this->renderer->Update();
}

void Scene::UpdateScene() {
    for (Camera& cam : this->cameras) {
        cam.Update(this->input);
    }

    for (SceneObject& obj : this->objects) {
        obj.Update();
    }
}
