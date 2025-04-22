#include "Renderer.hpp"
#include "LightManager.hpp"
#include "ReadFile.hpp"
#include "SceneObject.hpp"

Renderer::Renderer() {}

Renderer::~Renderer() {
    ID3D11Debug* debug = nullptr;
    if (SUCCEEDED(this->device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debug)))) {
        debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL); // D3D11_RLDO_IGNORE_INTERNAL
        debug->Release();
    }
}

HRESULT Renderer::Init(const Window& window) {
    this->SetViewPort(window);
    HRESULT result = this->CreateDeviceAndSwapChain(window);
    if (FAILED(result)) return result;

    result = this->rr.Init(this->device.Get(), window.GetWidth(), window.GetHeight());
    if (FAILED(result)) return result;

    result = CreateUAV();
    if (FAILED(result)) return result;

    result = this->SetupRasterizerStates();
    if (FAILED(result)) return result;

    std::string byteData;
    result = this->SetShaders(byteData);
    if (FAILED(result)) return result;

    result = this->SetInputLayout(byteData);
    if (FAILED(result)) return result;

    result = this->SetSamplers();
    if (FAILED(result)) return result;

    this->metadataBuffer.Initialize(this->device.Get(), sizeof(CSMetadata), nullptr);
    this->viewProjBuffer.Initialize(this->device.Get(), sizeof(DirectX::XMFLOAT4X4) * 3, nullptr);
    this->cameraBuffer.Initialize(this->device.Get(), sizeof(CameraBufferData), nullptr);
    // this->viewPos.Initialize(this->device.Get(), sizeof(DirectX::XMVECTOR), nullptr);
    this->tessBuffer.Initialize(this->device.Get(), sizeof(TessellationData), nullptr);

    return S_OK;
}

void Renderer::Render(BaseScene* scene) {
    // this->BindLights(scene.getLights());
    LightManager& lm = scene->GetLightManager();
    this->ShadowPass(scene->GetLightManager(), scene->GetObjects());
    lm.BindDepthTextures(this->GetDeviceContext(), 5, 6);
    lm.BindLightData(this->GetDeviceContext(), 7, 8);

    std::array<float, 4> clearColor{0, 0, 0, 1};
    for (int i = 0; i < this->renderPasses; i++) {
        for (auto& cam : scene->GetCameras()) {
            this->Render(scene, *cam, cam->GetAdressOfUAV(), cam->GetRenderResources());
            // clear
            cam->GetRenderResources()->Clear(this->immediateContext.Get(), clearColor);
        }
    }
    // Render to backbuffer
    this->Render(scene, scene->GetMainCam(), this->UAV.GetAddressOf(), &this->rr);

    // clear
    this->rr.Clear(this->immediateContext.Get(), clearColor);
    // lm.UnbindDepthTextures(this->immediateContext.Get(), 3);

    // Present
    this->swapChain->Present(1, 0);
}

ID3D11Device* Renderer::GetDevice() { return this->device.Get(); }

ID3D11DeviceContext* Renderer::GetDeviceContext() const { return this->immediateContext.Get(); }

void Renderer::Render(BaseScene* scene, Camera& cam, ID3D11UnorderedAccessView** UAV, RenderingResources* rr) {
    D3D11_VIEWPORT vp = rr->GetViewPort();
    this->immediateContext->RSSetViewports(1, &vp);

    rr->BindGeometryPass(this->GetDeviceContext());
    this->BindViewAndProjMatrixes(cam);

    // Bind and update camera buffer

    // Tessellation ON
    this->immediateContext->HSSetShader(this->hullShader.Get(), nullptr, 0);
    this->immediateContext->HSSetConstantBuffers(0, 1, this->tessBuffer.GetAdressOfBuffer());
    this->immediateContext->DSSetShader(this->domainShader.Get(), nullptr, 0);
    this->immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
    this->immediateContext->PSSetConstantBuffers(1, 1, this->cameraBuffer.GetAdressOfBuffer());
    CameraBufferData camdata{};
    DirectX::XMStoreFloat4x4(&camdata.viewProjection,
                             DirectX::XMMatrixMultiplyTranspose(cam.createViewMatrix(), cam.createProjectionMatrix()));
    XMStoreFloat3(&camdata.cameraPosition, cam.transform.GetPosition());
    this->cameraBuffer.UpdateBuffer(this->immediateContext.Get(), &camdata);

    // Draw objects / Bind objects
    for (SceneObject*& obj : scene->GetVisibleObjects(cam)) {
        // Calculate distance to object from camera
        float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(
            DirectX::XMVectorSubtract(obj->transform.GetPosition(), cam.transform.GetPosition())));
        this->tessBuffer.UpdateBuffer(this->GetDeviceContext(), &distance);

        // Draw object
        obj->Draw(this->device.Get(), this->immediateContext.Get());
    }

    // Tessellation OFF
    this->immediateContext->HSSetShader(nullptr, nullptr, 0);
    this->immediateContext->DSSetShader(nullptr, nullptr, 0);
    this->immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Draw bounding boxes with wireframe fill mode
    this->immediateContext->RSSetState(this->wireframeRasterizerState.Get());
    for (SceneObject*& box : scene->GetBoundingBoxes()) {
        box->Draw(this->device.Get(), this->immediateContext.Get());
    }
    this->immediateContext->RSSetState(this->solidRasterizerState.Get());

    // Render particles using the particle system
    scene->GetParticleSystem().UpdateParticles(this->device.Get(), this->immediateContext.Get(), 0.016f);
    this->RenderParticles(scene->GetParticleSystem(), cam, rr);

    this->immediateContext->CSSetShader(this->computeShader.Get(), nullptr, 0);
    this->BindLightMetaData(cam, static_cast<int>(scene->GetLightManager().GetSpotLights().size()),
                            static_cast<int>(scene->GetLightManager().GetDirectionalLights().size()));
    rr->BindLightingPass(this->GetDeviceContext());
    // Do lighting pass
    this->LightingPass(UAV, vp);
}

void Renderer::ShadowPass(LightManager& lm, std::vector<SceneObject*> obj) {
    this->GetDeviceContext()->PSSetShader(nullptr, nullptr, 0);

    const std::vector<Light>& SpotLights = lm.GetSpotLights();
    this->GetDeviceContext()->RSSetViewports(1, &lm.GetSpotLightVP());

    for (const Light& light : SpotLights) {
        this->immediateContext->ClearDepthStencilView(light.GetDepthStencilVeiw(), D3D11_CLEAR_DEPTH, 1, 0);
        this->BindShadowViewAndProjection(light.CreateViewMatrix(), light.CreateProjectionMatrix());
        this->GetDeviceContext()->OMSetRenderTargets(0, nullptr, light.GetDepthStencilVeiw());
        for (auto& obj : obj) {
            obj->Draw(this->device.Get(), this->immediateContext.Get());
        }
    }

    const std::vector<DirectionalLight>& DirLights = lm.GetDirectionalLights();
    this->GetDeviceContext()->RSSetViewports(1, &lm.GetDirectionalLightVP());
    for (const DirectionalLight& light : DirLights) {
        this->immediateContext->ClearDepthStencilView(light.GetDepthStencilVeiw(), D3D11_CLEAR_DEPTH, 1, 0);
        this->BindShadowViewAndProjection(light.CreateViewMatrix(), light.CreateProjectionMatrix());
        this->GetDeviceContext()->OMSetRenderTargets(0, nullptr, light.GetDepthStencilVeiw());
        for (auto& obj : obj) {
            obj->Draw(this->device.Get(), this->immediateContext.Get());
        }
    }

    this->GetDeviceContext()->OMSetRenderTargets(0, nullptr, this->rr.GetDepthStencilView());
    this->GetDeviceContext()->PSSetShader(this->pixelShader.Get(), nullptr, 0);
    this->GetDeviceContext()->CSSetShaderResources(3, SpotLights.size(), lm.GetAdressOfSpotlightDSSRV());
}

void Renderer::BindShadowViewAndProjection(DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix) {
    // Create and Bind view and projection matrixes
    DirectX::XMFLOAT4X4 matrices[3];
    DirectX::XMStoreFloat4x4(&matrices[0], viewMatrix);
    DirectX::XMStoreFloat4x4(&matrices[1], projectionMatrix);
    DirectX::XMStoreFloat4x4(&matrices[2], DirectX::XMMatrixMultiplyTranspose(viewMatrix, projectionMatrix));

    this->viewProjBuffer.UpdateBuffer(this->immediateContext.Get(), matrices);
    this->immediateContext.Get()->VSSetConstantBuffers(0, 1, this->viewProjBuffer.GetAdressOfBuffer());
}

void Renderer::Clear() {
    std::array<float, 4> clearColor{0, 0, 0, 1};

    this->rr.Clear(this->immediateContext.Get(), clearColor);
}

HRESULT Renderer::CreateDeviceAndSwapChain(const Window& window) {
    DXGI_SWAP_CHAIN_DESC swapChainDesc               = {};
    swapChainDesc.BufferCount                        = 2;
    swapChainDesc.BufferDesc.Width                   = 0;
    swapChainDesc.BufferDesc.Height                  = 0;
    swapChainDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator   = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.SampleDesc.Count                   = 1;
    swapChainDesc.SampleDesc.Quality                 = 0;
    swapChainDesc.BufferUsage                        = DXGI_USAGE_UNORDERED_ACCESS | DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow                       = window.GetHWND();
    swapChainDesc.Windowed                           = TRUE;
    swapChainDesc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags                              = 0;

    return D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG, nullptr,
                                         0, D3D11_SDK_VERSION, &swapChainDesc, this->swapChain.GetAddressOf(),
                                         this->device.GetAddressOf(), nullptr, this->immediateContext.GetAddressOf());
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

    // Pixel Shader
    if (!CM::ReadFile("DCEMPS.cso", shaderData)) {
        std::cerr << "Failed to read pixel shader file!" << std::endl;
        return E_FAIL;
    }

    result = this->device->CreatePixelShader(shaderData.data(), shaderData.size(), nullptr,
                                             this->pixelShaderDCEM.GetAddressOf());
    if (FAILED(result)) {
        std::cerr << "Failed to create pixel shader DCEM!" << std::endl;
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

    // Hull Shader
    shaderData.clear();
    if (!CM::ReadFile("TessellationHS.cso", shaderData)) {
        std::cerr << "Failed to read hull shader file!" << std::endl;
        return E_FAIL;
    }

    result =
        this->device->CreateHullShader(shaderData.data(), shaderData.size(), nullptr, this->hullShader.GetAddressOf());
    if (FAILED(result)) {
        std::cerr << "Failed to create hull shader!" << std::endl;
        return result;
    }

    // Domain Shader
    shaderData.clear();
    if (!CM::ReadFile("TessellationDS.cso", shaderData)) {
        std::cerr << "Failed to read domain shader file!" << std::endl;
        return E_FAIL;
    }

    result = this->device->CreateDomainShader(shaderData.data(), shaderData.size(), nullptr,
                                              this->domainShader.GetAddressOf());
    if (FAILED(result)) {
        std::cerr << "Failed to create domain shader!" << std::endl;
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
        .ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY,
        .Texture2DArray{
            .MipSlice        = 0,
            .FirstArraySlice = 0,
            .ArraySize       = 1,
        },
    };

    result = this->device->CreateRenderTargetView(backbuffer.Get(), nullptr, this->rtv.GetAddressOf());
    if (FAILED(result)) {
        return result;
    }

    return this->device->CreateUnorderedAccessView(backbuffer.Get(), &desc, this->UAV.GetAddressOf());
}

ID3D11PixelShader* Renderer::GetPS() const { return this->pixelShader.Get(); }

ID3D11PixelShader* Renderer::GetDCEMPS() const { return this->pixelShaderDCEM.Get(); }

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
    this->immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

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
        .Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
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
    this->immediateContext->CSSetSamplers(0, 1, this->samplerState.GetAddressOf());

    return S_OK;
}

HRESULT Renderer::SetupRasterizerStates() {
    // Solid rendering
    D3D11_RASTERIZER_DESC solidDesc{
        .FillMode              = D3D11_FILL_SOLID,
        .CullMode              = D3D11_CULL_BACK,
        .FrontCounterClockwise = FALSE,
        .DepthClipEnable       = TRUE,
    };
    HRESULT result = this->device->CreateRasterizerState(&solidDesc, this->solidRasterizerState.GetAddressOf());
    if (FAILED(result)) return result;

    // Wireframe rendering
    D3D11_RASTERIZER_DESC wireframeDesc = {
        .FillMode              = D3D11_FILL_WIREFRAME,
        .CullMode              = D3D11_CULL_BACK,
        .FrontCounterClockwise = FALSE,
        .DepthClipEnable       = TRUE,
    };
    result = this->device->CreateRasterizerState(&wireframeDesc, this->wireframeRasterizerState.GetAddressOf());
    if (FAILED(result)) return result;

    // Set solid rendering as default
    this->immediateContext->RSSetState(this->solidRasterizerState.Get());

    return S_OK;
}

void Renderer::LightingPass(ID3D11UnorderedAccessView** UAV, D3D11_VIEWPORT viewport) {
    // Unbind GBuffers from writing
    // this->rr.BindLightingPass(this->immediateContext.Get());

    // Bind UAV to Compute

    this->immediateContext->CSSetUnorderedAccessViews(0, 1, UAV, nullptr);

    // Do Compute
    this->immediateContext->Dispatch(viewport.Width / 8, viewport.Height / 8, 1);

    ID3D11UnorderedAccessView* resetter[1] = {nullptr};

    // Unbind UAV from Compute
    this->immediateContext->CSSetUnorderedAccessViews(0, 1, resetter, nullptr);
}

void Renderer::BindLightMetaData(const Camera& cam, int nrOfLights, int nrOfDirLights) {
    float* pos = cam.transform.GetPosition().m128_f32;
    CSMetadata metaData{
        .nrofLights = nrOfLights, .nrofDirLights = nrOfDirLights, .cameraPos = {pos[0], pos[1], pos[2]}};

    this->metadataBuffer.UpdateBuffer(this->immediateContext.Get(), &metaData);
    this->immediateContext.Get()->CSSetConstantBuffers(0, 1, this->metadataBuffer.GetAdressOfBuffer());
}

void Renderer::RenderParticles(ParticleSystem& particleSystem, Camera& cam, RenderingResources* rr) {
    this->immediateContext->IASetInputLayout(nullptr);
    this->immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    this->immediateContext->VSSetShader(particleSystem.GetVertexShader(), nullptr, 0);
    this->immediateContext->VSSetShaderResources(0, 1, particleSystem.GetAddressOfSRV());

    CameraBufferData cameraData;
    DirectX::XMStoreFloat4x4(&cameraData.viewProjection,
                             DirectX::XMMatrixMultiplyTranspose(cam.createViewMatrix(), cam.createProjectionMatrix()));
    DirectX::XMFLOAT3 camPos;
    DirectX::XMStoreFloat3(&camPos, cam.transform.GetPosition());
    cameraData.cameraPosition = camPos;
    cameraBuffer.UpdateBuffer(this->immediateContext.Get(), &cameraData);
    this->immediateContext->GSSetShader(particleSystem.GetGeometryShader(), nullptr, 0);
    this->immediateContext->GSSetConstantBuffers(1, 1, cameraBuffer.GetAdressOfBuffer());

    D3D11_VIEWPORT viewport = rr->GetViewPort();
    this->immediateContext->RSSetViewports(1, &viewport);
    this->immediateContext->PSSetShader(particleSystem.GetPixelShader(), nullptr, 0);

    // this->immediateContext->OMSetRenderTargets(1, this->rtv.GetAddressOf(), this->rr.GetDepthStencilView());
    this->immediateContext->Draw(particleSystem.GetParticleCount(), 0);

    this->immediateContext->GSSetShader(nullptr, nullptr, 0);
    this->immediateContext->VSSetShader(this->vertexShader.Get(), nullptr, 0);
    this->immediateContext->PSSetShader(this->pixelShader.Get(), nullptr, 0);
    this->immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    this->immediateContext->IASetInputLayout(this->inputLayout.Get());

    ID3D11ShaderResourceView* nullSRV[1] = {nullptr};
    this->immediateContext->VSSetShaderResources(0, 1, nullSRV);
}

void Renderer::BindViewAndProjMatrixes(const Camera& cam) {
    // Create and Bind view and projection matrixes
    DirectX::XMFLOAT4X4 matrices[3];
    DirectX::XMMATRIX viewMatrix       = cam.createViewMatrix();
    DirectX::XMMATRIX projectionMatrix = cam.createProjectionMatrix();
    DirectX::XMStoreFloat4x4(&matrices[0], viewMatrix);
    DirectX::XMStoreFloat4x4(&matrices[1], projectionMatrix);
    DirectX::XMStoreFloat4x4(&matrices[2], DirectX::XMMatrixMultiplyTranspose(viewMatrix, projectionMatrix));

    this->viewProjBuffer.UpdateBuffer(this->immediateContext.Get(), matrices);
    this->immediateContext.Get()->VSSetConstantBuffers(0, 1, this->viewProjBuffer.GetAdressOfBuffer());
    this->immediateContext.Get()->DSSetConstantBuffers(0, 1, this->viewProjBuffer.GetAdressOfBuffer());
}
