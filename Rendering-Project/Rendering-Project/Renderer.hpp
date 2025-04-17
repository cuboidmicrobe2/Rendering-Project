#ifndef RENDERER
#define RENDERER
#include "WindowHandler.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include <d3d11_4.h>
#include "Gbuffer.hpp"
class Renderer {
  public:
    Renderer();

    HRESULT Init(const Window& window);

    void Render(Scene& scene);

    ID3D11Device* GetDevice();

  private:

    void Clear();

    void Render(Scene& scene, Camera& cam, ID3D11UnorderedAccessView** UAV, D3D11_VIEWPORT& viewport);

    HRESULT CreateDeviceAndSwapChain(const Window& window);
    HRESULT CreateDepthStencil(const Window& window);
    HRESULT SetInputLayout(const std::string& byteCode);
    void SetViewPort(const Window& window);
    HRESULT SetSamplers();

    void LightingPass(ID3D11UnorderedAccessView** UAV);
    void BindLights(const std::vector<Light>& lights);
    void BindViewAndProjMatrixes(const Camera& cam);
    void BindLightMetaData(const Camera& cam, int nrOfLights);

    HRESULT SetShaders(std::string& byteDataOutput);
    HRESULT CreateUAV();

    const uint32_t renderPasses = 1;
    D3D11_VIEWPORT viewport;

    
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShaderDCEM;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> computeShader;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> UAV;

    GBuffer position;
    GBuffer color;
    GBuffer normal;
};

#endif