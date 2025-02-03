#include "Renderer.hpp"

Renderer::Renderer() {}

Renderer::~Renderer() {}

HRESULT Renderer::Init(HWND hWnd) {
    HRESULT result = CreateDeviceAndSwapChain(hWnd);
    if (FAILED(result))
        return result;

    result = CreateRenderTarget();
    if (FAILED(result))
        return result;

    result = CreateDepthStencil();
    if (FAILED(result))
        return result;

    SetViewPort();
    return S_OK;
}

void Renderer::Reset() {
    if (renderTargetView)
        renderTargetView.Reset();
    if (depthStencilView)
        depthStencilView.Reset();
    if (swapChain)
        swapChain.Reset();
    if (immediateContext)
        immediateContext.Reset();
    if (device)
        device.Reset();
}

void Renderer::Present() { swapChain->Present(1, 0); }

HRESULT Renderer::CreateDeviceAndSwapChain(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {
        .BufferCount = 1, 
        .BufferDesc.Width = 0)
    }
    return E_NOTIMPL;
}

HRESULT Renderer::CreateRenderTarget() { return E_NOTIMPL; }

HRESULT Renderer::CreateDepthStencil() { return E_NOTIMPL; }

void Renderer::SetViewPort() {}
