#ifndef RENDERER
#define RENDERER
#include "BaseScene.hpp"
#include "Camera.hpp"
#include "Gbuffer.hpp"
#include "LightManager.hpp"
#include "MeshHandler.hpp"
#include "StructuredBuffer.hpp"
#include "WindowHandler.hpp"
#include <d3d11_4.h>

class Renderer {
  public:
    Renderer();
    ~Renderer();

    HRESULT Init(const Window& window);

    void Render(BaseScene* scene, float deltaTime);

    ID3D11Device* GetDevice();
    ID3D11PixelShader* GetPS() const;
    ID3D11PixelShader* GetDCEMPS() const;
    ID3D11DeviceContext* GetDeviceContext() const;

    MeshHandler meshHandler;

    Microsoft::WRL::ComPtr<ID3D11Device> GetDeviceCOMPTR() { return this->device; }

  private:
    void Clear();

    void Render(BaseScene* scene, Camera* cam, ID3D11UnorderedAccessView** UAV, RenderingResources* rr,
                float deltaTime);

    HRESULT CreateDeviceAndSwapChain(const Window& window);
    HRESULT SetInputLayout(const std::string& byteCode);
    void SetViewPort(const Window& window);
    HRESULT SetSamplers();
    HRESULT SetupRasterizerStates();

    void LightingPass(ID3D11UnorderedAccessView** UAV, D3D11_VIEWPORT viewport);
    void BindViewAndProjMatrixes(const Camera& cam);
    void BindLightMetaData(const Camera& cam, int nrOfLights, int nrOfDirLights);
    void RenderParticles(ParticleSystem& particleSystem, Camera& cam, RenderingResources* rr);
    void ShadowPass(LightManager& lm, std::vector<SceneObject*> obj);
    void BindShadowViewAndProjection(DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix);

    void SetTesselation(bool value, bool visibility);
    bool tesselationStatus      = false;
    bool tessellationVisibility = false;

    HRESULT SetShaders(std::string& byteDataOutput);
    HRESULT CreateUAV();

    const uint32_t renderPasses = 1;
    D3D11_VIEWPORT viewport;

    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;

    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerStateShadows;

    Microsoft::WRL::ComPtr<ID3D11RasterizerState> wireframeRasterizerState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> solidRasterizerState;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShaderDCEM;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> computeShader;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> UAV;
    Microsoft::WRL::ComPtr<ID3D11HullShader> hullShader;
    Microsoft::WRL::ComPtr<ID3D11DomainShader> domainShader;

    RenderingResources rr;

    static constexpr int MAX_LIGHTS = 32;

    struct LightData {
        float pos[3];
        float intensity;
        float color[4];
        float direction[3];
        float angle;
    };

    struct CSMetadata {
        int nrofLights;
        int nrofDirLights;
        float padding[2];
        float cameraPos[3];
        float alsoPadding;
    };

    struct CameraBufferData {
        DirectX::XMFLOAT4X4 viewProjection;
        DirectX::XMFLOAT3 cameraPosition;
        float padding;
    };

    struct TessellationData {
        float distance;
        float padding[3];
    };

    struct RenderingMode {
        int mode = 0;
        float _padding[3]{};
    } renderingMode;

    ConstantBuffer metadataBuffer;
    ConstantBuffer viewProjBuffer;
    ConstantBuffer cameraBuffer;
    ConstantBuffer tessBuffer;
    ConstantBuffer worldMatrixBuffer;
    ConstantBuffer renderingModeBuffer;
};

#endif