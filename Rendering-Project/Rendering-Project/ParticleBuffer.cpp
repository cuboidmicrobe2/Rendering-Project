#include "ParticleBuffer.hpp"

ParticleBuffer::ParticleBuffer(Microsoft::WRL::ComPtr<ID3D11Device> device) {
    device = this->device;
    this->Create();
}

ParticleBuffer::~ParticleBuffer() {}

HRESULT ParticleBuffer::Create() {
    D3D11_BUFFER_DESC desc = {
        .ByteWidth = this->size * this->nrOf,
        .Usage     = this->dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE,
        .BindFlags = (this->hasSRV ? D3D11_BIND_SHADER_RESOURCE : 0) | (this->hasUAV ? D3D11_BIND_UNORDERED_ACCESS : 0),
        .CPUAccessFlags      = this->dynamic ? D3D11_CPU_ACCESS_WRITE : 0,
        .MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
        .StructureByteStride = this->size,
    };

    // Create buffer
    ID3D11Buffer* buffer = nullptr;
    HRESULT result       = this->device->CreateBuffer(&desc, nullptr, &buffer);
    if (FAILED(result)) {
        return result;
    }

    // Create the SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {
        .Format        = DXGI_FORMAT_UNKNOWN,
        .ViewDimension = D3D11_SRV_DIMENSION_BUFFER,
        .Buffer{
            .ElementWidth = this->nrOf,
        },
    };

    ID3D11ShaderResourceView* srv = nullptr;
    result                        = this->device->CreateShaderResourceView(buffer, &srvDesc, &srv);
    if (FAILED(result)) {
        buffer->Release();
        return result;
    }

    // Create the UAV
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {
        .Format        = DXGI_FORMAT_UNKNOWN,
        .ViewDimension = D3D11_UAV_DIMENSION_BUFFER,
        .Buffer{
            .NumElements = this->nrOf,
        },
    };

    ID3D11UnorderedAccessView* uav = nullptr;
    result                         = this->device->CreateUnorderedAccessView(buffer, &uavDesc, &uav);
    if (FAILED(result)) {
        srv->Release();
        uav->Release();
        return result;
    }

    return S_OK;
}