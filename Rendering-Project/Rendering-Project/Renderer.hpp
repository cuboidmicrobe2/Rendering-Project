#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>

class Renderer {
  public:
    Renderer();
    ~Renderer();

    HRESULT Init(HWND hWnd);
    void Reset();
    void Present();

  private:
    HRESULT CreateDeviceAndSwapChain(HWND hWnd);
    HRESULT CreateRenderTarget();
    HRESULT CreateDepthStencil();
    void SetViewPort();

    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
    D3D11_VIEWPORT viewport;
};

#endif