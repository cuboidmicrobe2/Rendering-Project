#include "Renderer.hpp"
#include "ConstantBuffer.hpp"
#include "ReadFile.hpp"
#include "SimpleVertex.hpp"
#include "VertexBuffer.hpp"
#include <vector>

Renderer::Renderer(Window& window) : window(&window), viewport({}) {}

HRESULT Renderer::Init() {

    HRESULT result = this->CreateDeviceAndSwapChain();
    if (FAILED(result)) return result;

    result = this->CreateRenderTarget();
    if (FAILED(result)) return result;

    result = this->CreateDepthStencil();
    if (FAILED(result)) return result;

    this->SetViewPort();

    result = this->pipeline.Init(this->device, this->immediateContext);
    if (FAILED(result)) return result;

    result = this->CreateCube();
    if (FAILED(result)) return result;

    return S_OK;
}

void Renderer::Update() {
    this->Clear();
    this->Present();
}

void Renderer::Clear() {
    float clearColor[] = {1.0f, 0.5f, 0.2f, 1.0f};
    this->immediateContext->ClearRenderTargetView(this->renderTargetView.Get(), clearColor);
    this->immediateContext->ClearDepthStencilView(this->depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Renderer::Present() {
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    this->immediateContext->IASetVertexBuffers(0, 1, this->vertexBuffer.GetAddressOf(), &stride, &offset);

    immediateContext->RSSetViewports(1, &this->viewport);
    this->immediateContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), this->depthStencilView.Get());

    this->immediateContext->Draw(4, 0);

    this->swapChain->Present(1, 0);
}

HRESULT Renderer::CreateDeviceAndSwapChain() {
    DXGI_SWAP_CHAIN_DESC swapChainDesc               = {};
    swapChainDesc.BufferCount                        = 1;
    swapChainDesc.BufferDesc.Width                   = 0;
    swapChainDesc.BufferDesc.Height                  = 0;
    swapChainDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator   = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.SampleDesc.Count                   = 1;
    swapChainDesc.SampleDesc.Quality                 = 0;
    swapChainDesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow                       = this->window->GetHWND();
    swapChainDesc.Windowed                           = TRUE;
    swapChainDesc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags                              = 0;

    return D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION,
                                         &swapChainDesc, this->swapChain.GetAddressOf(), this->device.GetAddressOf(),
                                         nullptr, this->immediateContext.GetAddressOf());
}

HRESULT Renderer::CreateRenderTarget() {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backbuffer;
    HRESULT result =
        this->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backbuffer.GetAddressOf()));
    if (FAILED(result)) return result;

    return this->device->CreateRenderTargetView(backbuffer.Get(), nullptr, this->renderTargetView.GetAddressOf());
}

HRESULT Renderer::CreateDepthStencil() {
    D3D11_TEXTURE2D_DESC depthStencilDesc = {depthStencilDesc.Width              = window->GetWidth(),
                                             depthStencilDesc.Height             = window->GetHeight(),
                                             depthStencilDesc.MipLevels          = 1,
                                             depthStencilDesc.ArraySize          = 1,
                                             depthStencilDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT,
                                             depthStencilDesc.SampleDesc.Count   = 1,
                                             depthStencilDesc.SampleDesc.Quality = 0,
                                             depthStencilDesc.Usage              = D3D11_USAGE_DEFAULT,
                                             depthStencilDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL,
                                             depthStencilDesc.CPUAccessFlags     = 0,
                                             depthStencilDesc.MiscFlags          = 0};

    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencil;
    HRESULT result = device->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf());
    if (FAILED(result)) return result;

    return device->CreateDepthStencilView(depthStencil.Get(), nullptr, this->depthStencilView.GetAddressOf());
}

HRESULT Renderer::CreateCube() {

    SimpleVertex vertices[] = {
        {{-0.5f, 0.5f, 0.0f}, {0, 0, -1}, {0, 0}},
        {{0.5f, 0.5f, 0.0f}, {0, 0, -1}, {1, 0}},
        {{-0.5f, -0.5f, 0.0f}, {0, 0, -1}, {0, 1}},
        {{0.5f, -0.5f, 0.0f}, {0, 0, -1}, {1, 1}},
    };

    VertexBuffer vertexBuffer(this->device.Get(), sizeof(SimpleVertex), sizeof(vertices) / sizeof(SimpleVertex),
                              vertices);
    this->vertexBuffer = vertexBuffer.GetBuffer();
    if (!this->vertexBuffer) {
        return E_FAIL;
    }

    return S_OK;
}

void Renderer::SetViewPort() {
    this->viewport.Width    = static_cast<FLOAT>(window->GetWidth());
    this->viewport.Height   = static_cast<FLOAT>(window->GetHeight());
    this->viewport.MinDepth = 0.0f;
    this->viewport.MaxDepth = 1.0f;
    this->viewport.TopLeftX = 0;
    this->viewport.TopLeftY = 0;
}

Renderer::Pipeline::Pipeline() {}

HRESULT Renderer::Pipeline::Init(Microsoft::WRL::ComPtr<ID3D11Device> device,
                                 Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext) {
    this->device           = device;
    this->immediateContext = immediateContext;

    HRESULT result = this->SetShaders();
    if (FAILED(result)) return result;

    result = this->SetInputLayout();
    if (FAILED(result)) return result;

    result = this->SetDepthStencilState();
    if (FAILED(result)) return result;

    result = this->SetShaderResources();
    if (FAILED(result)) return result;

    result = this->SetSamplers();
    if (FAILED(result)) return result;

    return S_OK;
}

HRESULT Renderer::Pipeline::SetShaders() {
    std::string shaderData;

    // Vertex Shader
    if (!ReadFile("VertexShader.cso", shaderData)) {
        std::cerr << "Failed to read vertex shader file!" << std::endl;
        return E_FAIL;
    }

    HRESULT result = this->device->CreateVertexShader(shaderData.data(), shaderData.size(), nullptr,
                                                      this->vertexShader.GetAddressOf());
    if (FAILED(result)) {
        std::cerr << "Failed to create vertex shader!" << std::endl;
        return result;
    }

    this->byteCode = shaderData;
    shaderData.clear();

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

HRESULT Renderer::Pipeline::SetInputLayout() {
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}};

    HRESULT result = this->device->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc), this->byteCode.data(),
                                                     this->byteCode.size(), this->inputLayout.GetAddressOf());
    if (FAILED(result)) {
        return result;
    }

    this->immediateContext->IASetInputLayout(this->inputLayout.Get());
    this->immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    return S_OK;
}

HRESULT Renderer::Pipeline::SetDepthStencilState() { return S_OK; }

HRESULT Renderer::Pipeline::SetShaderResources() { return S_OK; }

HRESULT Renderer::Pipeline::SetSamplers() {
    D3D11_SAMPLER_DESC samplerDesc = {.Filter         = D3D11_FILTER_ANISOTROPIC,
                                      .AddressU       = D3D11_TEXTURE_ADDRESS_WRAP,
                                      .AddressV       = D3D11_TEXTURE_ADDRESS_WRAP,
                                      .AddressW       = D3D11_TEXTURE_ADDRESS_WRAP,
                                      .MipLODBias     = 0,
                                      .MaxAnisotropy  = 16,
                                      .ComparisonFunc = D3D11_COMPARISON_ALWAYS,
                                      .BorderColor    = {0, 0, 0, 0},
                                      .MinLOD         = 0.0f,
                                      .MaxLOD         = D3D11_FLOAT32_MAX};

    HRESULT result = device->CreateSamplerState(&samplerDesc, this->samplerState.GetAddressOf());
    if (FAILED(result)) {
        return result;
    }

    this->immediateContext->PSSetSamplers(0, 1, this->samplerState.GetAddressOf());

    return S_OK;
}