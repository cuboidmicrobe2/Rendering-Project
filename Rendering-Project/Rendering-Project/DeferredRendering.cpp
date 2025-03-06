#include "DeferredRendering.hpp"
#include "ReadFile.hpp"

DeferredRendering::DeferredRendering(Window& window) : DaddyRenderer(window) {}

DeferredRendering::~DeferredRendering() {}

HRESULT DeferredRendering::Init() {
    HRESULT result = this->CreateDeviceAndSwapChain();
    if (FAILED(result)) return result;

    result = this->CreateRenderTarget();
    if (FAILED(result)) return result;

    result = this->CreateDepthStencil();
    if (FAILED(result)) return result;

    this->SetViewPort();

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

void DeferredRendering::Update() {
    // do de deferred rendering : )
    // run compute shader, However that is done
    this->SecondPass();
    this->Present();
    this->Clear();
}

void DeferredRendering::SecondPass() {
    // Unbind GBuffers from writing
    this->immediateContext->OMSetRenderTargets(0, nullptr, nullptr); // ?
    ID3D11ShaderResourceView* SRVs[3] = {
        this->position.GetSRV(),
        this->color.GetSRV(),
        this->normal.GetSRV(),
    };
   
    // We still need a compute shader somewhere?
    this->immediateContext->CSSetShaderResources(0, 3, SRVs);

    // reset rendertargets
    ID3D11RenderTargetView* rendertargets[3] = {
        this->position.GetRTV(),
        this->color.GetRTV(),
        this->normal.GetRTV(),
    };
    this->immediateContext->OMSetRenderTargets(3, rendertargets, this->depthStencilView.Get());
}

HRESULT DeferredRendering::SetShaders(std::string& byteDataOutput) {
    // Vertex Shader
    if (!ReadFile("deferredVS.cso", byteDataOutput)) {
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
    if (!ReadFile("deferredPS.cso", shaderData)) {
        std::cerr << "Failed to read pixel shader file!" << std::endl;
        return E_FAIL;
    }

    result = this->device->CreatePixelShader(shaderData.data(), shaderData.size(), nullptr,
                                             this->pixelShader.GetAddressOf());
    if (FAILED(result)) {
        std::cerr << "Failed to create pixel shader!" << std::endl;
        return result;
    }

    this->immediateContext->VSSetShader(this->vertexShader.Get(), nullptr, 0);
    this->immediateContext->PSSetShader(this->pixelShader.Get(), nullptr, 0);

    return S_OK;
}
