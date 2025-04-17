#include "Renderer.hpp"
#include "ReadFile.hpp"

Renderer::Renderer() {}

HRESULT Renderer::Init(const Window& window) {
    HRESULT result = this->CreateDeviceAndSwapChain(window);
    if (FAILED(result)) return result;

    this->position.Init(this->device.Get(), window.GetWidth(), window.GetHeight());
    this->normal.Init(this->device.Get(), window.GetWidth(), window.GetHeight());
    this->color.Init(this->device.Get(), window.GetWidth(), window.GetHeight());

    result = CreateUAV();
    if (FAILED(result)) return result;

    result = this->CreateDepthStencil(window);
    if (FAILED(result)) return result;

    this->SetViewPort(window);

    std::string byteData;
    result = this->SetShaders(byteData);
    if (FAILED(result)) return result;

    result = this->SetInputLayout(byteData);
    if (FAILED(result)) return result;

    result = this->SetSamplers();
    if (FAILED(result)) return result;

    ID3D11RenderTargetView* rendertargets[3] = {
        this->position.GetRTV(),
        this->color.GetRTV(),
        this->normal.GetRTV(),
    };
    this->immediateContext->OMSetRenderTargets(3, rendertargets, this->depthStencilView.Get());

    return S_OK;
}

void Renderer::Render(Scene& scene) {
    // this->BindLights(scene.getLights());
    //  Bind Lights
    const std::vector<Light>& lights = scene.getLights();
    ConstantBuffer lightBuffer;
    int nrOfLights = static_cast<int>(lights.size());
    struct LightData {
        float pos[3];
        float intensity;
        float color[4];
    };
    std::vector<LightData> lightData;
    for (auto light : lights) {
        float* tempPos   = light.transform.GetPosition().m128_f32;
        float* tempColor = light.GetColor().m128_f32;

        LightData l{
            .pos{tempPos[0], tempPos[1], tempPos[2]},
            .intensity = light.GetIntesity(),
            .color     = {tempColor[0], tempColor[1], tempColor[2], tempColor[3]},
        };
        lightData.emplace_back(l);
    }
    lightBuffer.Initialize(this->device.Get(), lightData.size() * sizeof(LightData), lightData.data());
    this->immediateContext.Get()->CSSetConstantBuffers(1, 1, lightBuffer.GetAdressOfBuffer());

    // Render all extra cameras to their texures
    for (auto& cam : scene.getCameras()) {
        this->Render(scene, cam, cam.GetAdressOfUAV());
    }

    // Render to backbuffer
    this->Render(scene, scene.getMainCam(), this->UAV.GetAddressOf());

    // Present
    this->immediateContext->RSSetViewports(1, &this->viewport);
    this->swapChain->Present(1, 0);
}

ID3D11Device* Renderer::GetDevice() { return this->device.Get(); }

ID3D11DeviceContext* Renderer::GetDeviceContext() const { return this->immediateContext.Get(); }

void Renderer::Render(Scene& scene, Camera& cam, ID3D11UnorderedAccessView** UAV) {
    // Render particles using the particle system
    scene.GetParticleSystem().UpdateParticles(this->device.Get(), this->immediateContext.Get(), 0.016f);
    this->RenderParticles(scene.GetParticleSystem(), cam);

    this->immediateContext->VSSetShader(this->vertexShader.Get(), nullptr, 0);
    this->immediateContext->PSSetShader(this->pixelShader.Get(), nullptr, 0);
    this->immediateContext->CSSetShader(this->computeShader.Get(), nullptr, 0);

    this->BindViewAndProjMatrixes(cam);

    this->BindLightMetaData(cam, static_cast<int>(scene.getLights().size()));

    // Draw objects / Bind objects
    for (auto obj : scene.getObjects()) {
        obj.Draw(this->device.Get(), this->immediateContext.Get());
    }

    // Do lighting pass
    this->LightingPass(UAV);

    // clear
    this->Clear();
}

void Renderer::Clear() {
    float clearColor[] = {1.0f, 0.5f, 0.2f, 1.0f};
    float zero[4]{};

    this->immediateContext->ClearRenderTargetView(this->color.GetRTV(), clearColor);
    this->immediateContext->ClearRenderTargetView(this->position.GetRTV(), zero);
    this->immediateContext->ClearRenderTargetView(this->normal.GetRTV(), zero);
    this->immediateContext->ClearDepthStencilView(this->depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

HRESULT Renderer::CreateDeviceAndSwapChain(const Window& window) {
    DXGI_SWAP_CHAIN_DESC swapChainDesc               = {};
    swapChainDesc.BufferCount                        = 1;
    swapChainDesc.BufferDesc.Width                   = 0;
    swapChainDesc.BufferDesc.Height                  = 0;
    swapChainDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator   = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.SampleDesc.Count                   = 1;
    swapChainDesc.SampleDesc.Quality                 = 0;
    swapChainDesc.BufferUsage                        = DXGI_USAGE_UNORDERED_ACCESS;
    swapChainDesc.OutputWindow                       = window.GetHWND();
    swapChainDesc.Windowed                           = TRUE;
    swapChainDesc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags                              = 0;

    return D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION,
                                         &swapChainDesc, this->swapChain.GetAddressOf(), this->device.GetAddressOf(),
                                         nullptr, this->immediateContext.GetAddressOf());
}

HRESULT Renderer::SetShaders(std::string& byteDataOutput) {
    if (!CM::ReadFile("deferredVS.cso", byteDataOutput)) {
        std::cerr << "Failed to read vertex shader file!" << std::endl;
        return E_FAIL;
    }

    HRESULT result = this->device->CreateVertexShader(byteDataOutput.data(), byteDataOutput.size(), nullptr,
                                                      this->vertexShader.GetAddressOf());
    if (FAILED(result)) {
        std::cerr << "Failed to create vertex shader!" << std::endl;
        return result;
    }

    std::string shaderData;

    // Pixel Shader
    if (!CM::ReadFile("deferredPS.cso", shaderData)) {
        std::cerr << "Failed to read pixel shader file!" << std::endl;
        return E_FAIL;
    }

    result = this->device->CreatePixelShader(shaderData.data(), shaderData.size(), nullptr,
                                             this->pixelShader.GetAddressOf());
    if (FAILED(result)) {
        std::cerr << "Failed to create pixel shader!" << std::endl;
        return result;
    }

    // Compute Shader
    shaderData.clear();
    if (!CM::ReadFile("ComputeShader.cso", shaderData)) {
        std::cerr << "Failed to read Compute shader file!" << std::endl;
        return E_FAIL;
    }

    result = this->device->CreateComputeShader(shaderData.data(), shaderData.size(), nullptr,
                                               this->computeShader.GetAddressOf());
    if (FAILED(result)) {
        std::cerr << "Failed to create Compute shader!" << std::endl;
        return result;
    }

    this->immediateContext->VSSetShader(this->vertexShader.Get(), nullptr, 0);
    this->immediateContext->PSSetShader(this->pixelShader.Get(), nullptr, 0);
    this->immediateContext->CSSetShader(this->computeShader.Get(), nullptr, 0);

    return S_OK;
}

HRESULT Renderer::CreateUAV() { // Vertex Shader
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backbuffer;
    HRESULT result =
        this->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backbuffer.GetAddressOf()));
    if (FAILED(result)) return result;

    D3D11_UNORDERED_ACCESS_VIEW_DESC desc{
        .Format        = DXGI_FORMAT_R8G8B8A8_UNORM,
        .ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D,
        .Texture2D{
            .MipSlice = 0,
        },
    };

    return this->device->CreateUnorderedAccessView(backbuffer.Get(), &desc, this->UAV.GetAddressOf());
}

HRESULT Renderer::CreateDepthStencil(const Window& window) {
    D3D11_TEXTURE2D_DESC depthStencilDesc = {
        depthStencilDesc.Width              = window.GetWidth(),
        depthStencilDesc.Height             = window.GetHeight(),
        depthStencilDesc.MipLevels          = 1,
        depthStencilDesc.ArraySize          = 1,
        depthStencilDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT,
        depthStencilDesc.SampleDesc.Count   = 1,
        depthStencilDesc.SampleDesc.Quality = 0,
        depthStencilDesc.Usage              = D3D11_USAGE_DEFAULT,
        depthStencilDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL,
        depthStencilDesc.CPUAccessFlags     = 0,
        depthStencilDesc.MiscFlags          = 0,
    };

    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencil;
    HRESULT result = device->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf());
    if (FAILED(result)) return result;

    return device->CreateDepthStencilView(depthStencil.Get(), nullptr, this->depthStencilView.GetAddressOf());
}

HRESULT Renderer::SetInputLayout(const std::string& byteCode) {
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}};

    HRESULT result = this->device->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc), byteCode.data(),
                                                     byteCode.size(), this->inputLayout.GetAddressOf());
    if (FAILED(result)) {
        return result;
    }

    this->immediateContext->IASetInputLayout(this->inputLayout.Get());
    this->immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return S_OK;
}

void Renderer::SetViewPort(const Window& window) {
    this->viewport.Width    = static_cast<FLOAT>(window.GetWidth());
    this->viewport.Height   = static_cast<FLOAT>(window.GetHeight());
    this->viewport.MinDepth = 0.0f;
    this->viewport.MaxDepth = 1.0f;
    this->viewport.TopLeftX = 0;
    this->viewport.TopLeftY = 0;
}

HRESULT Renderer::SetSamplers() {
    D3D11_SAMPLER_DESC samplerDesc = {
        .Filter         = D3D11_FILTER_ANISOTROPIC,
        .AddressU       = D3D11_TEXTURE_ADDRESS_WRAP,
        .AddressV       = D3D11_TEXTURE_ADDRESS_WRAP,
        .AddressW       = D3D11_TEXTURE_ADDRESS_WRAP,
        .MipLODBias     = 0,
        .MaxAnisotropy  = 16,
        .ComparisonFunc = D3D11_COMPARISON_ALWAYS,
        .BorderColor    = {0, 0, 0, 0},
        .MinLOD         = 0.0f,
        .MaxLOD         = D3D11_FLOAT32_MAX,
    };

    HRESULT result = device->CreateSamplerState(&samplerDesc, this->samplerState.GetAddressOf());
    if (FAILED(result)) {
        return result;
    }

    this->immediateContext->PSSetSamplers(0, 1, this->samplerState.GetAddressOf());

    return S_OK;
}

void Renderer::LightingPass(ID3D11UnorderedAccessView** UAV) {
    // Unbind GBuffers from writing
    this->immediateContext->OMSetRenderTargets(0, nullptr, nullptr); // ?

    // Bind gbuffers to Compute
    ID3D11ShaderResourceView* SRVs[3] = {
        this->position.GetSRV(),
        this->color.GetSRV(),
        this->normal.GetSRV(),
    };
    this->immediateContext->CSSetShaderResources(0, 3, SRVs);

    // Bind UAV to Compute
    this->immediateContext->CSSetUnorderedAccessViews(0, 1, UAV, nullptr);

    // Do Compute
    this->immediateContext->Dispatch(240, 135, 1);

    ID3D11UnorderedAccessView* resetter[1] = {nullptr};

    // Unbind UAV from Compute
    this->immediateContext->CSSetUnorderedAccessViews(0, 1, resetter, nullptr);

    ID3D11ShaderResourceView* rVResetter[3] = {nullptr, nullptr, nullptr};
    // Unbind Gbuffers from Compute
    this->immediateContext->CSSetShaderResources(0, 3, rVResetter);

    // reset rendertargets
    ID3D11RenderTargetView* rendertargets[3] = {
        this->position.GetRTV(),
        this->color.GetRTV(),
        this->normal.GetRTV(),
    };
    this->immediateContext->OMSetRenderTargets(3, rendertargets, this->depthStencilView.Get());
}

void Renderer::BindLights(const std::vector<Light>& lights) {
    // Bind Lights
    ConstantBuffer lightBuffer;
    int nrOfLights = static_cast<int>(lights.size());
    struct LightData {
        float pos[3];
        float intensity;
        float color[4];
    };
    std::vector<LightData> lightData;
    for (auto light : lights) {
        float* tempPos   = light.transform.GetPosition().m128_f32;
        float* tempColor = light.GetColor().m128_f32;

        LightData l{
            .pos{tempPos[0], tempPos[1], tempPos[2]},
            .intensity = light.GetIntesity(),
            .color     = {tempColor[0], tempColor[1], tempColor[2], tempColor[3]},
        };
        lightData.emplace_back(l);
    }
    lightBuffer.Initialize(this->device.Get(), lightData.size() * sizeof(LightData), lightData.data());
    this->immediateContext.Get()->CSSetConstantBuffers(1, 1, lightBuffer.GetAdressOfBuffer());
}

void Renderer::BindLightMetaData(const Camera& cam, int nrOfLights) {
    struct CSMetadata {
        int nrofLights;
        float cameraPos[3];
    };

    float* t = cam.transform.GetPosition().m128_f32;
    CSMetadata metaData{.nrofLights = nrOfLights, .cameraPos = {t[0], t[1], t[2]}};

    ConstantBuffer CSMetaData;
    CSMetaData.Initialize(this->device.Get(), sizeof(metaData), &metaData);
    this->immediateContext.Get()->CSSetConstantBuffers(0, 1, CSMetaData.GetAdressOfBuffer());
}

void Renderer::RenderParticles(ParticleSystem& particleSystem, Camera& cam) {
    this->immediateContext->VSSetShader(particleSystem.GetVertexShader(), nullptr, 0);
    // this->immediateContext->PSSetShader(particleSystem.GetPixelShader(), nullptr, 0);
    // this->immediateContext->GSSetShader(particleSystem.GetGeometryShader(), nullptr, 0);

    struct CameraBufferData {
        DirectX::XMFLOAT4X4 viewProjection;
        DirectX::XMFLOAT3 cameraPosition;
        float padding;
    };

    CameraBufferData cameraData;
    DirectX::XMStoreFloat4x4(&cameraData.viewProjection,
                             DirectX::XMMatrixMultiplyTranspose(cam.createViewMatrix(), cam.createProjectionMatrix()));

    // Get camera position
    DirectX::XMFLOAT3 camPos;
    DirectX::XMStoreFloat3(&camPos, cam.transform.GetPosition());
    cameraData.cameraPosition = camPos;

    // Create and initialize the constant buffer
    ConstantBuffer cameraBuffer;
    cameraBuffer.Initialize(this->device.Get(), sizeof(CameraBufferData), &cameraData);

    // Bind the camera buffer to the geometry shader at register b1
    ID3D11Buffer* camBufferPtr = cameraBuffer.GetBuffer();
    this->immediateContext->GSSetConstantBuffers(1, 1, &camBufferPtr);

    ID3D11ShaderResourceView* srv = particleSystem.GetSRV();
    this->immediateContext->VSSetShaderResources(0, 1, &srv);

    this->immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    particleSystem.SetInputLayout(this->GetDeviceContext());

    UINT nrOfParticles = particleSystem.GetParticleCount();
    this->immediateContext->Draw(nrOfParticles, 0);

    this->immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    this->immediateContext->IASetInputLayout(this->inputLayout.Get());

    ID3D11ShaderResourceView* nullSRV[1] = {nullptr};
    this->immediateContext->VSSetShaderResources(0, 1, nullSRV);
}

void Renderer::BindViewAndProjMatrixes(const Camera& cam) {
    // Create and Bind view and projection matrixes
    ConstantBuffer viewAndProjectionMatrices;
    DirectX::XMFLOAT4X4 matrices[3];
    DirectX::XMMATRIX viewMatrix       = cam.createViewMatrix();
    DirectX::XMMATRIX projectionMatrix = cam.createProjectionMatrix();
    DirectX::XMStoreFloat4x4(&matrices[0], viewMatrix);
    DirectX::XMStoreFloat4x4(&matrices[1], projectionMatrix);
    DirectX::XMStoreFloat4x4(&matrices[2], DirectX::XMMatrixMultiplyTranspose(viewMatrix, projectionMatrix));
    viewAndProjectionMatrices.Initialize(this->device.Get(), sizeof(matrices), matrices);
    this->immediateContext.Get()->VSSetConstantBuffers(0, 1, viewAndProjectionMatrices.GetAdressOfBuffer());
}
