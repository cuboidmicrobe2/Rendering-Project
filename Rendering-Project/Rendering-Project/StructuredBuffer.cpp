#include "StructuredBuffer.hpp"
#include <stdexcept>

StructuredBuffer::StructuredBuffer(ID3D11Device* device, UINT sizeOfElement, size_t nrOfElementsInBuffer,
                                   void* bufferData, bool dynamic) {
    this->Initialize(device, sizeOfElement, nrOfElementsInBuffer, bufferData, dynamic);
}

StructuredBuffer::~StructuredBuffer() {
    if (this->buffer) {
        this->buffer->Release();
    }
    if (this->srv) {
        this->srv->Release();
    }
}

void StructuredBuffer::Initialize(ID3D11Device* device, UINT sizeOfElement, size_t nrOfElementsInBuffer,
                                  void* bufferData, bool dynamic) {
    this->elementSize  = sizeOfElement;
    this->nrOfElements = nrOfElementsInBuffer;

    D3D11_BUFFER_DESC bufferDesc = {
        .ByteWidth           = sizeOfElement * static_cast<UINT>(nrOfElementsInBuffer),
        .Usage               = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT,
        .BindFlags           = D3D11_BIND_SHADER_RESOURCE,
        .CPUAccessFlags      = static_cast<UINT> (dynamic ? D3D11_CPU_ACCESS_WRITE : 0),
        .MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
        .StructureByteStride = sizeOfElement,
    };

    D3D11_SUBRESOURCE_DATA initData = {
        .pSysMem = bufferData,
    };

    HRESULT hr = device->CreateBuffer(&bufferDesc, bufferData ? &initData : nullptr, &this->buffer);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create structured buffer");
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {.Format        = DXGI_FORMAT_UNKNOWN,
                                               .ViewDimension = D3D11_SRV_DIMENSION_BUFFER,
                                               .Buffer{
                                                   .ElementWidth = static_cast<UINT>(nrOfElementsInBuffer),
                                               }

    };

    hr = device->CreateShaderResourceView(this->buffer, &srvDesc, &this->srv);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create shader resource view for structured buffer");
    }
}

void StructuredBuffer::UpdateBuffer(ID3D11DeviceContext* context, void* data) {
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = context->Map(this->buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to map structured buffer");
    }

    memcpy(mappedResource.pData, data, this->elementSize * this->nrOfElements);
    context->Unmap(this->buffer, 0);
}

UINT StructuredBuffer::GetElementSize() const { return this->elementSize; }

size_t StructuredBuffer::GetNrOfElements() const { return this->nrOfElements; }

ID3D11ShaderResourceView* StructuredBuffer::GetSRV() const { return this->srv; }

ID3D11ShaderResourceView** StructuredBuffer::GetAdressOfSRV() { return &this->srv; }
