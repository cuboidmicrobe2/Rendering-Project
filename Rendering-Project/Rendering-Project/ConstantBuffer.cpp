#include "ConstantBuffer.hpp"
#include <stdexcept>

ConstantBuffer::ConstantBuffer(ID3D11Device* device, size_t byteSize, void* initialData) : bufferSize(byteSize) {
    D3D11_BUFFER_DESC desc{
        .ByteWidth           = static_cast<UINT>(byteSize),
        .Usage               = D3D11_USAGE_IMMUTABLE,
        .BindFlags           = D3D11_BIND_CONSTANT_BUFFER,
        .CPUAccessFlags      = 0,
        .MiscFlags           = 0,
        .StructureByteStride = 0,
    };

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem          = initialData;
    data.SysMemPitch      = 0;
    data.SysMemSlicePitch = 0;
    if (FAILED(device->CreateBuffer(&desc, &data, &this->buffer)))
        throw std::runtime_error("Falied to create constant buffer");
}

// Pointer Train go Chu Chu!
ConstantBuffer::~ConstantBuffer() { this->buffer->Release(); }

ConstantBuffer::ConstantBuffer(ConstantBuffer&& other) noexcept {
    this->buffer     = other.buffer;
    this->bufferSize = other.bufferSize;
    other.buffer     = nullptr;
    other.bufferSize = 0;
}

ConstantBuffer& ConstantBuffer::operator=(ConstantBuffer&& other) noexcept {
    if (this != &other) {
        if (this->buffer) this->buffer->Release();
        this->buffer     = other.buffer;
        this->bufferSize = other.bufferSize;
        other.buffer     = nullptr;
        other.bufferSize = 0;
    }
    return *this;
}

void ConstantBuffer::Initialize(ID3D11Device* device, size_t byteSize, void* initialData) {
    this->bufferSize = byteSize;
    D3D11_BUFFER_DESC desc{
        .ByteWidth           = static_cast<UINT>(byteSize),
        .Usage               = D3D11_USAGE_DYNAMIC,
        .BindFlags           = D3D11_BIND_CONSTANT_BUFFER,
        .CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE,
        .MiscFlags           = 0,
        .StructureByteStride = 0,
    };

    D3D11_SUBRESOURCE_DATA data{
        .pSysMem          = initialData,
        .SysMemPitch      = 0,
        .SysMemSlicePitch = 0,

    };
    if (FAILED(device->CreateBuffer(&desc, &data, &this->buffer)))
        throw std::runtime_error("Falied to create constant buffer");
    if (this->buffer == nullptr) throw std::runtime_error("Falied to create constant buffer");
}

size_t ConstantBuffer::GetSize() const { return this->bufferSize; }

ID3D11Buffer* ConstantBuffer::GetBuffer() const { return this->buffer; }

ID3D11Buffer** ConstantBuffer::GetAdressOfBuffer() { return &this->buffer; }

void ConstantBuffer::UpdateBuffer(ID3D11DeviceContext* context, void* data) {
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(mappedResource));

    if (FAILED(context->Map(this->buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
        throw std::runtime_error("Failed to update buffer");

    memcpy(mappedResource.pData, data, this->bufferSize);
    context->Unmap(this->buffer, 0);
}
