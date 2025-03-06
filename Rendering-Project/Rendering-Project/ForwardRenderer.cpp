#include "ForwardRenderer.hpp"
#include "ReadFile.hpp"

ForwardRenderer::ForwardRenderer(Window& window) : DaddyRenderer(window) {}

ForwardRenderer::~ForwardRenderer() {}

HRESULT ForwardRenderer::Init() {
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

    return S_OK;
}

void ForwardRenderer::Update() {
    this->Present();
    this->Clear();
}

HRESULT ForwardRenderer::SetShaders(std::string& byteDataOutput) {
    // Vertex Shader
    if (!ReadFile("VertexShader.cso", byteDataOutput)) {
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
    if (!ReadFile("PixelShader.cso", shaderData)) {
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
