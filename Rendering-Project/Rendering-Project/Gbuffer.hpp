#ifndef G_BUFFER_HPP
#define G_BUFFER_HPP
#include <d3d11.h>
#include <stdexcept>
#include <wrl/client.h>

class GBuffer {
  public:
    GBuffer() = default;
    inline GBuffer(ID3D11Device* device, const UINT windowWidth, const UINT windowHeight);
    inline ~GBuffer();

    inline void Init(ID3D11Device* device, const UINT windowWidth, const UINT windowHeight);

    inline ID3D11Texture2D* GetTexture();
    inline ID3D11ShaderResourceView* GetSRV();
    inline ID3D11RenderTargetView* GetRTV();

  private:
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture      = nullptr;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv = nullptr;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv   = nullptr;
};

GBuffer::GBuffer(ID3D11Device* device, const UINT windowWidth, const UINT windowHeight) {
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width              = windowWidth;
    desc.Height             = windowHeight;
    desc.MipLevels          = 1;
    desc.ArraySize          = 1;
    desc.Format             = DXGI_FORMAT_R32G32B32A32_FLOAT;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage              = D3D11_USAGE_DEFAULT;
    desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    desc.CPUAccessFlags     = 0;
    desc.MiscFlags          = 0;
    HRESULT hr              = device->CreateTexture2D(&desc, nullptr, this->texture.GetAddressOf());
    if (FAILED(hr)) {
        throw std::runtime_error("Could not create G-buffer texture");
    }
    hr = device->CreateShaderResourceView(texture.Get(), nullptr, this->srv.GetAddressOf());
    if (FAILED(hr)) {
        throw std::runtime_error("Could not create G-buffer srv");
    }

    hr = device->CreateRenderTargetView(texture.Get(), nullptr, this->rtv.GetAddressOf());
    if (FAILED(hr)) {
        throw std::runtime_error("Could not create G-buffer rtv");
    }
}

inline GBuffer::~GBuffer() {}

inline void GBuffer::Init(ID3D11Device* device, const UINT windowWidth, const UINT windowHeight) {
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width              = windowWidth;
    desc.Height             = windowHeight;
    desc.MipLevels          = 1;
    desc.ArraySize          = 1;
    desc.Format             = DXGI_FORMAT_R32G32B32A32_FLOAT;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage              = D3D11_USAGE_DEFAULT;
    desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    desc.CPUAccessFlags     = 0;
    desc.MiscFlags          = 0;

    HRESULT hr = device->CreateTexture2D(&desc, nullptr, this->texture.GetAddressOf());
    if (FAILED(hr)) {
        throw std::runtime_error("Could not create G-buffer texture");
    }
    hr = device->CreateShaderResourceView(this->texture.Get(), nullptr, this->srv.GetAddressOf());
    if (FAILED(hr)) {
        throw std::runtime_error("Could not create G-buffer srv");
    }
    hr = device->CreateRenderTargetView(this->texture.Get(), nullptr, this->rtv.GetAddressOf());
    if (FAILED(hr)) {
        throw std::runtime_error("Could not create G-buffer rtv");
    }
}

inline ID3D11Texture2D* GBuffer::GetTexture() { return this->texture.Get(); }

inline ID3D11ShaderResourceView* GBuffer::GetSRV() { return this->srv.Get(); }

inline ID3D11RenderTargetView* GBuffer::GetRTV() { return this->rtv.Get(); }

#endif
