#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "WindowHandler.hpp"
#include <d3d11.h>
#include <wrl/client.h>

class Renderer {
  public:
    Renderer(Window& window);
    ~Renderer();

    HRESULT Init();
    void Reset();
    void Present();

  private:
    HRESULT CreateDeviceAndSwapChain();
    HRESULT CreateRenderTarget();
    HRESULT CreateDepthStencil();
    void SetViewPort();

    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
    D3D11_VIEWPORT viewport;
    Window* window;
};

#endif