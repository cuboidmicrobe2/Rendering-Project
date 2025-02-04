#include "DepthBuffer.hpp"
#include <stdexcept>

DepthBuffer::DepthBuffer(ID3D11Device* device, UINT width, UINT height, bool hasSRV) {
    Initialize(device, width, height, hasSRV);
}

DepthBuffer::~DepthBuffer() {
    if (texture) {
        texture->Release();
    }
    for (auto dsv : depthStencilViews) {
        if (dsv) {
            dsv->Release();
        }
    }
    if (srv) {
        srv->Release();
    }
}

void DepthBuffer::Initialize(ID3D11Device* device, UINT width, UINT height, bool hasSRV, UINT arraySize) {
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width                = width;
    textureDesc.Height               = height;
    textureDesc.MipLevels            = 1;
    textureDesc.ArraySize            = arraySize;
    textureDesc.Format               = DXGI_FORMAT_R24G8_TYPELESS;
    textureDesc.SampleDesc.Count     = 1;
    textureDesc.SampleDesc.Quality   = 0;
    textureDesc.Usage                = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags            = D3D11_BIND_DEPTH_STENCIL | (hasSRV ? D3D11_BIND_SHADER_RESOURCE : 0);
    textureDesc.CPUAccessFlags       = 0;
    textureDesc.MiscFlags            = 0;

    HRESULT hr = device->CreateTexture2D(&textureDesc, nullptr, &texture);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create depth stencil texture");
    }

    depthStencilViews.resize(arraySize);
    for (UINT i = 0; i < arraySize; ++i) {
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format                        = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsvDesc.ViewDimension = (arraySize > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DARRAY : D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2DArray.ArraySize       = 1;
        dsvDesc.Texture2DArray.FirstArraySlice = i;
        dsvDesc.Texture2DArray.MipSlice        = 0;

        hr = device->CreateDepthStencilView(texture, &dsvDesc, &depthStencilViews[i]);
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create depth stencil view");
        }
    }

    if (hasSRV) {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {.Format        = DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
                                                   .ViewDimension = (arraySize > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DARRAY
                                                                                    : D3D11_SRV_DIMENSION_TEXTURE2D,
                                                   .Texture2DArray{
                                                       .MostDetailedMip = 0,
                                                       .MipLevels       = 1,
                                                       .FirstArraySlice = 0,
                                                       .ArraySize       = arraySize,
                                                   }};

        hr = device->CreateShaderResourceView(texture, &srvDesc, &srv);
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create shader resource view");
        }
    }
}

ID3D11DepthStencilView* DepthBuffer::GetDSV(UINT arrayIndex) const {
    if (arrayIndex < this->depthStencilViews.size()) {
        return this->depthStencilViews[arrayIndex];
    }
    return nullptr;
}

ID3D11ShaderResourceView* DepthBuffer::GetSRV() const { return this->srv; }
