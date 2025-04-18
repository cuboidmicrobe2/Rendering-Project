#ifndef RENDERER
#define RENDERER
#include "Camera.hpp"
#include "Gbuffer.hpp"
#include "Scene.hpp"
#include "WindowHandler.hpp"
#include <d3d11_4.h>
class Renderer {
  public:
    Renderer();

    HRESULT Init(const Window& window);

    void Render(Scene& scene);

    ID3D11Device* GetDevice();
    ID3D11PixelShader* GetPS() const;
    ID3D11PixelShader* GetDCEMPS() const;
    ID3D11DeviceContext* GetDeviceContext() const;

  private:
    void Clear();

    void Render(Scene& scene, Camera& cam, ID3D11UnorderedAccessView** UAV, RenderingResources* rr);

    HRESULT CreateDeviceAndSwapChain(const Window& window);
    HRESULT SetInputLayout(const std::string& byteCode);
    void SetViewPort(const Window& window);
    HRESULT SetSamplers();

    void LightingPass(ID3D11UnorderedAccessView** UAV, D3D11_VIEWPORT viewport);
    void BindLights(const std::vector<Light>& lights);
    void BindViewAndProjMatrixes(const Camera& cam);
    void BindLightMetaData(const Camera& cam, int nrOfLights);
    void RenderParticles(ParticleSystem& particleSystem, Camera& cam);

    HRESULT SetShaders(std::string& byteDataOutput);
    HRESULT CreateUAV();


    const uint32_t renderPasses = 1;
    D3D11_VIEWPORT viewport;

    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShaderDCEM;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> computeShader;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> UAV;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
    Microsoft::WRL::ComPtr<ID3D11HullShader> hullShader;
    Microsoft::WRL::ComPtr<ID3D11DomainShader> domainShader;

    RenderingResources rr;

    static constexpr int MAX_LIGHTS = 16;

    struct LightData {
        float pos[3];
        float intensity;
        float color[4];
    };

    struct CSMetadata {
        int nrofLights;
        float cameraPos[3];
    };

    struct CameraBufferData {
        DirectX::XMFLOAT4X4 viewProjection;
        DirectX::XMFLOAT3 cameraPosition;
        float padding;
    };

    ConstantBuffer lightBuffer;
    ConstantBuffer metadataBuffer;
    ConstantBuffer viewProjBuffer;
    ConstantBuffer cameraBuffer;
    ConstantBuffer viewPos;
};

#endif