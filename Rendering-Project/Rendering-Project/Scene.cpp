#include "Scene.hpp"
#include <WICTextureLoader.h>
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

Scene::Scene(Window& window)
    : input(window.inputHandler), vertexShader(nullptr), geometryShader(nullptr), pixelShader(nullptr) {
    this->renderer = std::make_unique<DeferredRenderer>(window);
    if (FAILED(this->renderer->Init())) throw std::runtime_error("Failed to init renderer");
    this->particleBuffer.Create(this->renderer->GetDevice(), 32, 100, true, true, true);

    HRESULT hr;
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    hr = D3DCompileFromFile(L"Rendering-Project/ParticleVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG,
                            0, &vsBlob, nullptr);
    if (SUCCEEDED(hr)) {
        hr = this->renderer->GetDevice()->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
                                                             nullptr, &this->vertexShader);
    }

    Microsoft::WRL::ComPtr<ID3DBlob> gsBlob;
    hr = D3DCompileFromFile(L"Rendering-Project/ParticleGS.hlsl", nullptr, nullptr, "main", "gs_5_0", D3DCOMPILE_DEBUG,
                            0, &gsBlob, nullptr);
    if (SUCCEEDED(hr)) {
        hr = this->renderer->GetDevice()->CreateGeometryShader(gsBlob->GetBufferPointer(), gsBlob->GetBufferSize(),
                                                               nullptr, &this->geometryShader);
    }

    Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
    hr = D3DCompileFromFile(L"Rendering-Project/ParticlePS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG,
                            0, &psBlob, nullptr);
    if (SUCCEEDED(hr)) {
        hr = this->renderer->GetDevice()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
                                                            nullptr, &this->pixelShader);
    }

    Microsoft::WRL::ComPtr<ID3DBlob> csBlob;
    hr = D3DCompileFromFile(L"Rendering-Project/ParticleCS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
                            "cs_5_0", D3DCOMPILE_DEBUG, 0, &csBlob, &errorBlob);
    if (SUCCEEDED(hr)) {
        hr = this->renderer->GetDevice()->CreateComputeShader(csBlob->GetBufferPointer(), csBlob->GetBufferSize(),
                                                              nullptr, &this->computeShader);
    }

    // In Scene class constructor or Init method
    // Load particle texture
    hr = DirectX::CreateWICTextureFromFile(this->renderer->GetDevice(),
                                           L"slimepfp.jpg", // Adjust path to your texture
                                           nullptr, &this->srv);

    if (FAILED(hr)) {
        // Create a fallback white texture
        const UINT size = 64;
        std::vector<uint32_t> pixels(size * size, 0xFFFFFFFF);

        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width                = size;
        desc.Height               = size;
        desc.MipLevels            = 1;
        desc.ArraySize            = 1;
        desc.Format               = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count     = 1;
        desc.Usage                = D3D11_USAGE_DEFAULT;
        desc.BindFlags            = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA data = {};
        data.pSysMem                = pixels.data();
        data.SysMemPitch            = size * sizeof(uint32_t);

        Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
        hr = this->renderer->GetDevice()->CreateTexture2D(&desc, &data, &texture);
        if (SUCCEEDED(hr)) {
            this->renderer->GetDevice()->CreateShaderResourceView(texture.Get(), nullptr, &this->srv);
        }
    }
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

void Scene::RenderParticles(ParticleBuffer& particleBuffer, ID3D11VertexShader* vertexShader,
                            ID3D11GeometryShader* geometryShader, ID3D11PixelShader* pixelShader,
                            ID3D11ShaderResourceView* smokeTexture) {

    auto context = this->renderer->GetContext();

    // Set up the vertex shader
    context->VSSetShader(vertexShader, nullptr, 0);

    // Set up the geometry shader
    context->GSSetShader(geometryShader, nullptr, 0);

    // Set up the pixel shader
    context->PSSetShader(pixelShader, nullptr, 0);

    // Bind view and projection matrices (already set up in RenderScene)

    // Set the particle buffer as a shader resource view
    ID3D11ShaderResourceView* srv = particleBuffer.GetSRV();
    context->VSSetShaderResources(0, 1, &srv);

    // Set the smoke texture for the pixel shader
    context->PSSetShaderResources(0, 1, &smokeTexture);

    // Set up a sampler state for the texture
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter             = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU           = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV           = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW           = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc     = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD             = 0;
    samplerDesc.MaxLOD             = D3D11_FLOAT32_MAX;

    ID3D11SamplerState* samplerState;
    this->renderer->GetDevice()->CreateSamplerState(&samplerDesc, &samplerState);
    context->PSSetSamplers(0, 1, &samplerState);

    // Set up alpha blending
    D3D11_BLEND_DESC blendDesc                      = {};
    blendDesc.AlphaToCoverageEnable                 = FALSE;
    blendDesc.IndependentBlendEnable                = FALSE;
    blendDesc.RenderTarget[0].BlendEnable           = TRUE;
    blendDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    ID3D11BlendState* blendState;
    this->renderer->GetDevice()->CreateBlendState(&blendDesc, &blendState);
    context->OMSetBlendState(blendState, nullptr, 0xFFFFFFFF);

    // Draw particles - we're using point list topology
    // Each point will be expanded into a quad by the geometry shader
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    context->Draw(particleBuffer.GetNrOfElements(), 0);

    // Clean up
    context->GSSetShader(nullptr, nullptr, 0);
    context->VSSetShader(nullptr, nullptr, 0);
    context->PSSetShader(nullptr, nullptr, 0);

    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->VSSetShaderResources(0, 1, &nullSRV);
    context->PSSetShaderResources(0, 1, &nullSRV);

    // Release resources
    samplerState->Release();
    blendState->Release();
}

void Scene::UpdateParticles(ParticleBuffer& particleBuffer, ID3D11ComputeShader*& computeShader) {
    this->renderer->GetContext()->CSSetShader(computeShader, nullptr, 0);

    ID3D11UnorderedAccessView* uav = particleBuffer.GetUAV();
    this->renderer->GetContext()->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);

    this->renderer->GetContext()->Dispatch(std::ceil(particleBuffer.GetNrOfElements() / 32.0f), 1, 1);

    ID3D11UnorderedAccessView* nullUAV = nullptr;
    this->renderer->GetContext()->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
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
        float* tempPos   = light.transform.GetPosition().m128_f32;
        float* tempColor = light.GetColor().m128_f32;

        LightData l{
            .pos{tempPos[0], tempPos[1], tempPos[2]},
            .intensity = light.GetIntesity(),
            .color     = {tempColor[0], tempColor[1], tempColor[2], tempColor[3]},
        };
        lightData.emplace_back(l);
    }
    struct CSMetadata {
        int nrofLights;
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

    this->UpdateParticles(this->particleBuffer, this->computeShader);
    this->RenderParticles(this->particleBuffer, this->vertexShader, this->geometryShader, this->pixelShader, this->srv);

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
