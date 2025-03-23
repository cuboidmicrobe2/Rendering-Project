#include "DeferredRenderer.hpp"
#include "ReadFile.hpp"
#include "SimpleVertex.hpp"

DeferredRenderer::DeferredRenderer(Window& window) : DaddyRenderer(window) {}

DeferredRenderer::~DeferredRenderer() {}

HRESULT DeferredRenderer::Init() {
    HRESULT result = this->CreateDeviceAndSwapChain();
    if (FAILED(result)) return result;

    this->position.Init(this->GetDevice(), this->window->GetWidth(), this->window->GetHeight());
    this->normal.Init(this->GetDevice(), this->window->GetWidth(), this->window->GetHeight());
    this->color.Init(this->GetDevice(), this->window->GetWidth(), this->window->GetHeight());

    // result = this->CreateRenderTarget();
    // if (FAILED(result)) return result;

    result = CreateUAV();
    if (FAILED(result)) return result;

    result = this->CreateDepthStencil();
    if (FAILED(result)) return result;

    this->SetViewPort();

    std::string byteData;
    result = this->SetShaders(byteData);
    if (FAILED(result)) return result;

    result = this->SetInputLayout(byteData);
    if (FAILED(result)) return result;

    result = this->SetSamplers();
    if (FAILED(result)) return result;

    ID3D11RenderTargetView* rendertargets[3] = {
        this->position.GetRTV(),
        this->color.GetRTV(),
        this->normal.GetRTV(),
    };
    this->immediateContext->OMSetRenderTargets(3, rendertargets, this->depthStencilView.Get());

    return S_OK;
}

void DeferredRenderer::Update() {
    // do de deferred rendering : )
    // run compute shader, However that is done

    // Present
    this->SecondPass();
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;

    this->immediateContext->RSSetViewports(1, &this->viewport);

    this->swapChain->Present(1, 0);

    // this->Present();
    //  --

    
    // Clear
    float clearColor[] = {1.0f, 0.5f, 0.2f, 1.0f};
    float zero[4]{};

    this->immediateContext->ClearRenderTargetView(this->color.GetRTV(), clearColor);
    this->immediateContext->ClearRenderTargetView(this->position.GetRTV(), zero);
    this->immediateContext->ClearRenderTargetView(this->normal.GetRTV(), zero);
    this->immediateContext->ClearDepthStencilView(this->depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    // this->Clear();
    //  --
}

void DeferredRenderer::SecondPass() {
    // Unbind GBuffers from writing
    this->immediateContext->OMSetRenderTargets(0, nullptr, nullptr); // ?
    ID3D11ShaderResourceView* SRVs[3] = {
        this->position.GetSRV(),
        this->color.GetSRV(),
        this->normal.GetSRV(),
    };

    // Bind gbuffers to Compute
    this->immediateContext->CSSetShaderResources(0, 3, SRVs);

    // Bind UAV to Compute
    this->immediateContext->CSSetUnorderedAccessViews(0, 1, this->UAV.GetAddressOf(), nullptr);

    // Do Compute
    this->immediateContext->Dispatch(240, 135, 1);

    ID3D11UnorderedAccessView* resetter[1] = {nullptr};

    // Unbind UAV from Compute
    this->immediateContext->CSSetUnorderedAccessViews(0, 1, resetter, nullptr);

    ID3D11ShaderResourceView* rVResetter[3] = {nullptr, nullptr, nullptr};
    // Unbind Gbuffers from Compute
    this->immediateContext->CSSetShaderResources(0, 3, rVResetter);

    // reset rendertargets
    ID3D11RenderTargetView* rendertargets[3] = {
        this->position.GetRTV(),
        this->color.GetRTV(),
        this->normal.GetRTV(),
    };
    this->immediateContext->OMSetRenderTargets(3, rendertargets, this->depthStencilView.Get());
}

HRESULT DeferredRenderer::SetShaders(std::string& byteDataOutput) {
    // Vertex Shader
    if (!CM::ReadFile("deferredVS.cso", byteDataOutput)) {
        std::cerr << "Failed to read vertex shader file!" << std::endl;
        return E_FAIL;
    }

    HRESULT result = this->device->CreateVertexShader(byteDataOutput.data(), byteDataOutput.size(), nullptr,
                                                      this->vertexShader.GetAddressOf());
    if (FAILED(result)) {
        std::cerr << "Failed to create vertex shader!" << std::endl;
        return result;
    }

    std::string shaderData;

    // Pixel Shader
    if (!CM::ReadFile("deferredPS.cso", shaderData)) {
        std::cerr << "Failed to read pixel shader file!" << std::endl;
        return E_FAIL;
    }

    result = this->device->CreatePixelShader(shaderData.data(), shaderData.size(), nullptr,
                                             this->pixelShader.GetAddressOf());
    if (FAILED(result)) {
        std::cerr << "Failed to create pixel shader!" << std::endl;
        return result;
    }

    // Compute Shader
    shaderData.clear();
    if (!CM::ReadFile("ComputeShader.cso", shaderData)) {
        std::cerr << "Failed to read Compute shader file!" << std::endl;
        return E_FAIL;
    }

    result = this->device->CreateComputeShader(shaderData.data(), shaderData.size(), nullptr,
                                               this->computeShader.GetAddressOf());
    if (FAILED(result)) {
        std::cerr << "Failed to create Compute shader!" << std::endl;
        return result;
    }

    this->immediateContext->VSSetShader(this->vertexShader.Get(), nullptr, 0);
    this->immediateContext->PSSetShader(this->pixelShader.Get(), nullptr, 0);
    this->immediateContext->CSSetShader(this->computeShader.Get(), nullptr, 0);

    return S_OK;
}

HRESULT DeferredRenderer::CreateUAV() {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backbuffer;
    HRESULT result =
        this->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backbuffer.GetAddressOf()));
    if (FAILED(result)) return result;

    D3D11_UNORDERED_ACCESS_VIEW_DESC desc{
        .Format        = DXGI_FORMAT_R8G8B8A8_UNORM,
        .ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D,
        .Texture2D{
            .MipSlice = 0,
        },
    };

    return this->device->CreateUnorderedAccessView(backbuffer.Get(), &desc, this->UAV.GetAddressOf());
}

HRESULT DeferredRenderer::CreateDeviceAndSwapChain() {
    DXGI_SWAP_CHAIN_DESC swapChainDesc               = {};
    swapChainDesc.BufferCount                        = 1;
    swapChainDesc.BufferDesc.Width                   = 0;
    swapChainDesc.BufferDesc.Height                  = 0;
    swapChainDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator   = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.SampleDesc.Count                   = 1;
    swapChainDesc.SampleDesc.Quality                 = 0;
    swapChainDesc.BufferUsage                        = DXGI_USAGE_UNORDERED_ACCESS;
    swapChainDesc.OutputWindow                       = this->window->GetHWND();
    swapChainDesc.Windowed                           = TRUE;
    swapChainDesc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags                              = 0;

    return D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION,
                                         &swapChainDesc, this->swapChain.GetAddressOf(), this->device.GetAddressOf(),
                                         nullptr, this->immediateContext.GetAddressOf());
}
