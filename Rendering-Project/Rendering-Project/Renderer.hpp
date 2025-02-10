#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "WindowHandler.hpp"
#include <d3d11.h>
#include <wrl/client.h>

class Renderer {
  public:
    Renderer(Window& window);
    ~Renderer() = default;

    HRESULT Init();
    void Update();
    void Clear();

  private:
    HRESULT CreateDeviceAndSwapChain();
    HRESULT CreateRenderTarget();
    HRESULT CreateDepthStencil();
    HRESULT CreateCube();
    void SetViewPort();
    void Present();

    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    D3D11_VIEWPORT viewport;
    Window* window;

    class Pipeline {
      public:
        Pipeline();
        ~Pipeline() = default;

        HRESULT Init(Microsoft::WRL::ComPtr<ID3D11Device> device,
                     Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext);

      private:
        HRESULT SetShaders();
        HRESULT SetInputLayout();
        HRESULT SetDepthStencilState();
        HRESULT SetShaderResources();
        HRESULT SetSamplers();

        Microsoft::WRL::ComPtr<ID3D11Device> device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;

        Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
        std::string byteCode;

        Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;

        Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
    };

    Pipeline pipeline;
};

#endif