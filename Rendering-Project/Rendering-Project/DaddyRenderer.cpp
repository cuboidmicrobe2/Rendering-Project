#include "DaddyRenderer.hpp"
#include "SimpleVertex.hpp"

DaddyRenderer::DaddyRenderer(Window& window) : window(&window) {}

void DaddyRenderer::Clear() {
    float clearColor[] = {1.0f, 0.5f, 0.2f, 1.0f};
    this->immediateContext->ClearRenderTargetView(this->renderTargetView.Get(), clearColor);
    this->immediateContext->ClearDepthStencilView(this->depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void DaddyRenderer::Present() {
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;

    immediateContext->RSSetViewports(1, &this->viewport);
    this->immediateContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), this->depthStencilView.Get());

    this->swapChain->Present(1, 0);
}

HRESULT DaddyRenderer::CreateRenderTarget() {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backbuffer;
    HRESULT result =
        this->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backbuffer.GetAddressOf()));
    if (FAILED(result)) return result;

    return this->device->CreateRenderTargetView(backbuffer.Get(), nullptr, this->renderTargetView.GetAddressOf());
}

HRESULT DaddyRenderer::CreateDepthStencil() {
    D3D11_TEXTURE2D_DESC depthStencilDesc = {
        depthStencilDesc.Width              = window->GetWidth(),
        depthStencilDesc.Height             = window->GetHeight(),
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

HRESULT DaddyRenderer::SetInputLayout(const std::string& byteCode) {
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

void DaddyRenderer::SetViewPort() {
    this->viewport.Width    = static_cast<FLOAT>(window->GetWidth());
    this->viewport.Height   = static_cast<FLOAT>(window->GetHeight());
    this->viewport.MinDepth = 0.0f;
    this->viewport.MaxDepth = 1.0f;
    this->viewport.TopLeftX = 0;
    this->viewport.TopLeftY = 0;
}

HRESULT DaddyRenderer::SetSamplers() {
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
