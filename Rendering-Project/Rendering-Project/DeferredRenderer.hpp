#ifndef DEFERRED_RENDERER_HPP
#define DEFERRED_RENDERER_HPP

#include "DaddyRenderer.hpp"
#include "Gbuffer.hpp"

class DeferredRenderer : public DaddyRenderer {
  public:
    DeferredRenderer(Window& window);
    ~DeferredRenderer();

    HRESULT Init() override;
    void Update() override;

  private:
    void SecondPass();

    HRESULT SetShaders(std::string& byteDataOutput);
    HRESULT CreateUAV();
    HRESULT CreateDeviceAndSwapChain() override;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> computeShader;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> UAV;


    GBuffer position;
    GBuffer color;
    GBuffer normal;
};

#endif // !DEFERRED_RENDERING_HPP