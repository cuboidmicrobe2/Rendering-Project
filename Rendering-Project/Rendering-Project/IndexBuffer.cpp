#include "IndexBuffer.hpp"
#include <stdexcept>

IndexBuffer::IndexBuffer(ID3D11Device* device, size_t nrOfIndicesInBuffer, uint32_t* indexData)
    : nrOfIndices(nrOfIndicesInBuffer) {
    D3D11_BUFFER_DESC desc{
        .ByteWidth           = static_cast<UINT>(nrOfIndicesInBuffer * sizeof(*indexData)),
        .Usage               = D3D11_USAGE_IMMUTABLE,
        .BindFlags           = D3D11_BIND_INDEX_BUFFER,
        .CPUAccessFlags      = 0,
        .MiscFlags           = 0,
        .StructureByteStride = 0,
    };

    D3D11_SUBRESOURCE_DATA data{};
    data.pSysMem          = indexData;
    data.SysMemPitch      = 0;
    data.SysMemSlicePitch = 0;

    if (FAILED(device->CreateBuffer(&desc, &data, &this->buffer)))
        throw std::runtime_error("Failed to create index buffer");
}

// Pointer Train go Chu Chu!
IndexBuffer::~IndexBuffer() {
    if (this->buffer) this->buffer->Release();
}

IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept {
    this->buffer      = other.buffer;
    this->nrOfIndices = other.nrOfIndices;
    other.buffer      = nullptr;
    other.nrOfIndices = 0;
}

void IndexBuffer::Initialize(ID3D11Device* device, size_t nrOfIndicesInBuffer, uint32_t* indexData) {
    this->nrOfIndices = nrOfIndicesInBuffer;
    D3D11_BUFFER_DESC desc{
        .ByteWidth           = static_cast<UINT>(nrOfIndicesInBuffer * sizeof(*indexData)),
        .Usage               = D3D11_USAGE_IMMUTABLE,
        .BindFlags           = D3D11_BIND_INDEX_BUFFER,
        .CPUAccessFlags      = 0,
        .MiscFlags           = 0,
        .StructureByteStride = 0,
    };

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem          = indexData;
    data.SysMemPitch      = 0;
    data.SysMemSlicePitch = 0;

    if (FAILED(device->CreateBuffer(&desc, &data, &this->buffer)))
        throw std::runtime_error("Failed to create index buffer");
}

size_t IndexBuffer::GetNrOfIndices() const { return this->nrOfIndices; }

ID3D11Buffer* IndexBuffer::GetBuffer() const { return this->buffer; }
