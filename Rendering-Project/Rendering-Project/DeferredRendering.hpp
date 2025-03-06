#ifndef DEFERRED_RENDERING_HPP
#define DEFERRED_RENDERING_HPP

#include "DaddyRenderer.hpp"
#include "Gbuffer.hpp"


#pragma once
class DeferredRendering : public DaddyRenderer {
  public:
    DeferredRendering(Window& window);
    ~DeferredRendering();

    HRESULT Init() override;
    void Update() override;

  private:
    void SecondPass();

    HRESULT SetShaders(std::string& byteDataOutput);

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;

    GBuffer position;
    GBuffer color;
    GBuffer normal;
};
#endif // !DEFERRED_RENDERING_HPP
