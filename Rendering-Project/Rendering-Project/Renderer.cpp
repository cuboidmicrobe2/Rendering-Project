#include "Renderer.hpp"

Renderer::Renderer(Window& window) : window(&window), viewport({}) {}

Renderer::~Renderer() { this->Reset(); }

HRESULT Renderer::Init() {
    HRESULT result = this->CreateDeviceAndSwapChain();
    if (FAILED(result)) return result;

    result = this->CreateRenderTarget();
    if (FAILED(result)) return result;

    result = this->CreateDepthStencil();
    if (FAILED(result)) return result;

    this->SetViewPort();
    return S_OK;
}

void Renderer::Reset() {
    if (this->renderTargetView) this->renderTargetView.Reset();
    if (this->depthStencilView) this->depthStencilView.Reset();
    if (this->swapChain) this->swapChain.Reset();
    if (this->immediateContext) this->immediateContext.Reset();
    if (this->device) this->device.Reset();
}

void Renderer::Present() { this->swapChain->Present(1, 0); }

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

void Renderer::SetViewPort() {
    this->viewport.Width    = static_cast<FLOAT>(window->GetWidth());
    this->viewport.Height   = static_cast<FLOAT>(window->GetHeight());
    this->viewport.MinDepth = 0.0f;
    this->viewport.MaxDepth = 1.0f;
    this->viewport.TopLeftX = 0;
    this->viewport.TopLeftY = 0;
    immediateContext->RSSetViewports(1, &this->viewport);
}
