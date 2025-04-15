#ifndef DADDY_RENDERER_HPP
#define DADDY_RENDERER_HPP

#include "WindowHandler.hpp"
#include <d3d11.h>
#include <wrl/client.h>

class DaddyRenderer {
  public:
    DaddyRenderer(Window& window);
    virtual ~DaddyRenderer() = default;

    virtual HRESULT Init() = 0;
    virtual void Update() = 0;

    inline ID3D11Device* GetDevice() const { return this->device.Get(); };
    inline ID3D11DeviceContext* GetContext() const { return this->immediateContext.Get(); }

  protected:

    void Clear();
    void Present();

    virtual HRESULT CreateDeviceAndSwapChain() = 0;
    HRESULT CreateRenderTarget();
    HRESULT CreateDepthStencil();
    HRESULT SetInputLayout(const std::string& byteCode);
    void SetViewPort();
    HRESULT SetSamplers();

    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

    D3D11_VIEWPORT viewport;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
    Window* window;
};

#endif
