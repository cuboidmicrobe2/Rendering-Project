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
    if (!CM::ReadFile("VertexShader.cso", byteDataOutput)) {
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
    if (!CM::ReadFile("PixelShader.cso", shaderData)) {
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

HRESULT ForwardRenderer::CreateDeviceAndSwapChain() { 
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

    return D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
                                            D3D11_SDK_VERSION, &swapChainDesc, this->swapChain.GetAddressOf(),
                                            this->device.GetAddressOf(), nullptr,
                                            this->immediateContext.GetAddressOf());
}
