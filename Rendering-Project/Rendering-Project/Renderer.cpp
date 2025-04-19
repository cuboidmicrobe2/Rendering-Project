#include "Renderer.hpp"
#include "ReadFile.hpp"
#include "SceneObject.hpp"
#include "LightManager.hpp"

Renderer::Renderer() {}

HRESULT Renderer::Init(const Window& window) {
    this->SetViewPort(window);
    HRESULT result = this->CreateDeviceAndSwapChain(window);
    if (FAILED(result)) return result;

    result = this->rr.Init(this->device.Get(), window.GetWidth(), window.GetHeight());
    if (FAILED(result)) return result;

    result = CreateUAV();
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
    this->viewPos.Initialize(this->device.Get(), sizeof(DirectX::XMVECTOR), nullptr);
    this->tessBuffer.Initialize(this->device.Get(), sizeof(TessellationData), nullptr);

    return S_OK;
}

void Renderer::Render(Scene& scene) {
    //this->BindLights(scene.getLights());
    LightManager& lm = scene.GetLightManager();
    lm.BindLightData(this->GetDeviceContext(), 4);
    this->ShadowPass(scene.GetLightManager(), scene.getObjects());
    lm.BindDepthTextures(this->GetDeviceContext(), 3);

    DirectX::XMVECTOR pos = scene.getMainCam().transform.GetPosition();
    this->viewPos.UpdateBuffer(this->GetDeviceContext(), &pos);
    this->immediateContext->PSSetConstantBuffers(1, 1, this->viewPos.GetAdressOfBuffer());
    // Render all extra cameras to their texures
    std::array<float, 4> clearColor{0, 0, 0, 1};
    for (int i = 0; i < this->renderPasses; i++) {
        for (auto& cam : scene.getCameras()) {
            this->Render(scene, cam, cam.GetAdressOfUAV(), cam.GetRenderResources());
            // clear
            cam.GetRenderResources()->Clear(this->immediateContext.Get(), clearColor);
        }
    }
    // Render to backbuffer
    this->Render(scene, scene.getMainCam(), this->UAV.GetAddressOf(), &this->rr);

    // Render particles using the particle system
    scene.GetParticleSystem().UpdateParticles(this->device.Get(), this->immediateContext.Get(), 0.016f);
    this->RenderParticles(scene.GetParticleSystem(), scene.getMainCam());

    // clear
    this->rr.Clear(this->immediateContext.Get(), clearColor);
    lm.UnbindDepthTextures(this->immediateContext.Get(), 3);
    // Present
    this->swapChain->Present(1, 0);
}

ID3D11Device* Renderer::GetDevice() { return this->device.Get(); }

ID3D11DeviceContext* Renderer::GetDeviceContext() const { return this->immediateContext.Get(); }

void Renderer::Render(Scene& scene, Camera& cam, ID3D11UnorderedAccessView** UAV, RenderingResources* rr) {
    D3D11_VIEWPORT vp = rr->GetViewPort();
    this->immediateContext->RSSetViewports(1, &vp);

    rr->BindGeometryPass(this->GetDeviceContext());
    this->immediateContext->CSSetShader(this->computeShader.Get(), nullptr, 0);
    this->BindViewAndProjMatrixes(cam);
    this->BindLightMetaData(cam, static_cast<int>(scene.getLights().size()));

    // Tessellation ON
    this->immediateContext->HSSetShader(this->hullShader.Get(), nullptr, 0);
    this->immediateContext->HSSetConstantBuffers(0, 1, this->tessBuffer.GetAdressOfBuffer());
    this->immediateContext->DSSetShader(this->domainShader.Get(), nullptr, 0);
    this->immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

    // Draw objects / Bind objects
    for (auto& obj : scene.getObjects()) {
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

    rr->BindLightingPass(this->GetDeviceContext());
    // Do lighting pass
    this->LightingPass(UAV, viewport);
}

void Renderer::ShadowPass(LightManager& lm, std::vector<SceneObject*>& obj) {
    this->GetDeviceContext()->PSSetShader(nullptr, nullptr, 0);
    const std::vector<Light>& lights = lm.GetLights();
    this->GetDeviceContext()->RSSetViewports(1, &lm.GetViewPort());

    for (const Light& light : lights) {
        this->immediateContext->ClearDepthStencilView(light.GetDepthStencilVeiw(), D3D11_CLEAR_DEPTH, 1, 0);
        this->BindShadowViewAndProjection(light);
        this->GetDeviceContext()->OMSetRenderTargets(0, nullptr, light.GetDepthStencilVeiw());
        for (auto& obj : obj) {
            obj->Draw(this->device.Get(), this->immediateContext.Get());
        }
    }
    this->GetDeviceContext()->OMSetRenderTargets(0, nullptr, this->rr.GetDepthStencilView());
    this->GetDeviceContext()->PSSetShader(this->pixelShader.Get(), nullptr, 0);
    this->GetDeviceContext()->CSSetShaderResources(3, lights.size(), lm.GetAdressOfDSSRV());
}

void Renderer::BindShadowViewAndProjection(const Light& light) {
    // Create and Bind view and projection matrixes
    DirectX::XMFLOAT4X4 matrices[3];
    DirectX::XMMATRIX viewMatrix       = light.CreateViewMatrix();
    DirectX::XMMATRIX projectionMatrix = light.CreateProjectionMatrix();
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
    swapChainDesc.BufferCount                        = 1;
    swapChainDesc.BufferDesc.Width                   = 0;
    swapChainDesc.BufferDesc.Height                  = 0;
    swapChainDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator   = 60;
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
    this->immediateContext->CSSetSamplers(0, 1, this->samplerState.GetAddressOf());

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

//void Renderer::BindLights(const std::vector<Light>& lights) {
//    std::array<LightData, MAX_LIGHTS> lightData{};
//    for (int i = 0; i < lights.size(); i++) {
//        float* tempPos   = lights[i].transform.GetPosition().m128_f32;
//        float* tempColor = lights[i].GetColor().m128_f32;
//        float* tempDir   = lights[i].GetDirection().m128_f32;
//
//        LightData l{
//            .pos{tempPos[0], tempPos[1], tempPos[2]},
//            .intensity = lights[i].GetIntesity(),
//            .color     = {tempColor[0], tempColor[1], tempColor[2], tempColor[3]},
//            .direction = {tempDir[0], tempDir[1], tempDir[2]},
//            .angle     = cos(lights[i].GetAngle() / 2),
//        };
//        lightData[i] = l;
//    }
//
//    this->lightBuffer.UpdateBuffer(this->immediateContext.Get(), lightData.data());
//    lm.
//    this->immediateContext.Get()->CSSetConstantBuffers(1, 1, lm.);
//}

void Renderer::BindLightMetaData(const Camera& cam, int nrOfLights) {
    float* t = cam.transform.GetPosition().m128_f32;
    CSMetadata metaData{.nrofLights = nrOfLights, .cameraPos = {t[0], t[1], t[2]}};

    this->metadataBuffer.UpdateBuffer(this->immediateContext.Get(), &metaData);
    this->immediateContext.Get()->CSSetConstantBuffers(0, 1, this->metadataBuffer.GetAdressOfBuffer());
}

void Renderer::RenderParticles(ParticleSystem& particleSystem, Camera& cam) {
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

    this->immediateContext->RSSetViewports(1, &this->viewport);
    this->immediateContext->PSSetShader(particleSystem.GetPixelShader(), nullptr, 0);

    this->immediateContext->OMSetRenderTargets(1, this->rtv.GetAddressOf(), this->rr.GetDepthStencilView());
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
