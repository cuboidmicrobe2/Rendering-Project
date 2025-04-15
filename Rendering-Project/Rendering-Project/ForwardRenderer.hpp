#ifndef FORWARD_RENDERER_HPP
#define FORWARD_RENDERER_HPP

#include "DaddyRenderer.hpp"


class ForwardRenderer : public DaddyRenderer {
  public:
    ForwardRenderer(Window& window);
    ~ForwardRenderer();

    HRESULT Init() override;
    void Update() override;

  private:
    HRESULT SetShaders(std::string& byteDataOutput);
    HRESULT CreateDeviceAndSwapChain() override;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
};

#endif // !FORWARD_RENDERER_HPP