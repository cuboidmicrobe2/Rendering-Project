#include "ParticleBuffer.hpp"
#include <assert.h>
#include <iostream>

ParticleBuffer::ParticleBuffer() : size(0), nrOf(0), dynamic(false), buffer(nullptr), srv(nullptr), uav(nullptr) {}

UINT ParticleBuffer::GetNrOfElements() const { return this->nrOf; }

ID3D11Buffer* ParticleBuffer::GetBuffer() const { return this->buffer.Get(); }

ID3D11ShaderResourceView* ParticleBuffer::GetSRV() const { return this->srv.Get(); }

ID3D11ShaderResourceView** ParticleBuffer::GetSRVAddress() { return this->srv.GetAddressOf(); }

ID3D11UnorderedAccessView* ParticleBuffer::GetUAV() const { return this->uav.Get(); }

ID3D11UnorderedAccessView** ParticleBuffer::GetUAVAddress() { return this->uav.GetAddressOf(); }

HRESULT ParticleBuffer::Create(ID3D11Device* device, UINT size, UINT nrOf, bool dynamic, bool hasSRV, bool hasUAV,
                               void* initData) {
    // Store the parameters
    this->size    = size;
    this->nrOf    = nrOf;
    this->dynamic = dynamic;

    assert(nrOf != 0);
    assert(hasSRV | hasUAV != 0);
    assert(size >= 4 && size % 4 == 0);

    D3D11_BUFFER_DESC desc = {
        .ByteWidth = this->size * this->nrOf,
        .Usage     = this->dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT,
        .BindFlags = (hasSRV ? D3D11_BIND_SHADER_RESOURCE : static_cast<UINT>(0)) |
                     (hasUAV ? D3D11_BIND_UNORDERED_ACCESS : static_cast<UINT>(0)),
        .CPUAccessFlags      = this->dynamic ? D3D11_CPU_ACCESS_WRITE : static_cast<UINT>(0),
        .MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
        .StructureByteStride = this->size,
    };

    D3D11_SUBRESOURCE_DATA desc2{
        .pSysMem          = initData,
        .SysMemPitch      = 0,
        .SysMemSlicePitch = 0,
    };

    assert(desc.ByteWidth > 0);
    assert(desc.ByteWidth % desc.StructureByteStride == 0);

    // Create buffer
    HRESULT result = device->CreateBuffer(&desc, &desc2, this->buffer.GetAddressOf());
    if (FAILED(result)) {
        std::cerr << __LINE__ << result << "\n";
        return result;
    }

    // Create the SRV
    if (hasSRV) {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {
            .Format        = DXGI_FORMAT_UNKNOWN,
            .ViewDimension = D3D11_SRV_DIMENSION_BUFFER,
            .Buffer{
                .FirstElement = 0,
                .NumElements  = this->nrOf,
            },
        };

        result = device->CreateShaderResourceView(this->buffer.Get(), &srvDesc, this->srv.GetAddressOf());
        if (FAILED(result)) {
            std::cerr << __LINE__ << "\n";
            return result;
        }
    }

    // Create the UAV
    if (hasUAV) {
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {
            .Format        = DXGI_FORMAT_UNKNOWN,
            .ViewDimension = D3D11_UAV_DIMENSION_BUFFER,
            .Buffer{
                .FirstElement = 0,
                .NumElements  = this->nrOf,
                .Flags        = 0,
            },
        };

        result = device->CreateUnorderedAccessView(this->buffer.Get(), &uavDesc, this->uav.GetAddressOf());
        if (FAILED(result)) {
            std::cerr << __LINE__ << "\n";
            return result;
        }
    }

    return S_OK;
}
