#include "VertexBuffer.hpp"
#include <stdexcept>

VertexBuffer::VertexBuffer(ID3D11Device* device, UINT sizeOfVertex, UINT nrOfVerticesInBuffer, void* vertexData)
    : vertexSize(sizeOfVertex), nrOfVertices(nrOfVerticesInBuffer) {
    D3D11_BUFFER_DESC desc{
        .ByteWidth           = sizeOfVertex * nrOfVerticesInBuffer,
        .Usage               = D3D11_USAGE_IMMUTABLE,
        .BindFlags           = D3D11_BIND_VERTEX_BUFFER,
        .CPUAccessFlags      = 0,
        .MiscFlags           = 0,
        .StructureByteStride = 0,
    };

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem          = vertexData;
    data.SysMemPitch      = 0;
    data.SysMemSlicePitch = 0;

    if (FAILED(device->CreateBuffer(&desc, &data, this->buffer.GetAddressOf())))
        throw std::runtime_error("Failed to create Vertex buffer");
}

// Pointer Train go Chu Chu!
VertexBuffer::~VertexBuffer() { /*this->buffer->Release();*/ }

VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept {
    this->buffer       = std::move(other.buffer);
    this->nrOfVertices = other.nrOfVertices;
    this->vertexSize   = other.vertexSize;
    other.nrOfVertices = 0;
    other.vertexSize   = 0;
}

void VertexBuffer::Initialize(ID3D11Device* device, UINT sizeOfVertex, UINT nrOfVerticesInBuffer, void* vertexData) {
    this->vertexSize = sizeOfVertex;
    this->nrOfVertices = nrOfVerticesInBuffer;
    
    D3D11_BUFFER_DESC desc{
        .ByteWidth           = sizeOfVertex * nrOfVerticesInBuffer,
        .Usage               = D3D11_USAGE_IMMUTABLE,
        .BindFlags           = D3D11_BIND_VERTEX_BUFFER,
        .CPUAccessFlags      = 0,
        .MiscFlags           = 0,
        .StructureByteStride = 0,
    };

    D3D11_SUBRESOURCE_DATA data{
        .pSysMem          = vertexData,
        .SysMemPitch      = 0,
        .SysMemSlicePitch = 0,
    };

    if(FAILED(device->CreateBuffer(&desc, &data, &this->buffer))) 
        throw std::runtime_error("Failed to create Vertex buffer");
}

UINT VertexBuffer::GetNrOfVertices() const { return this->nrOfVertices; }

UINT VertexBuffer::GetVertexSize() const { return this->vertexSize; }

ID3D11Buffer* VertexBuffer::GetBuffer() const { return this->buffer.Get(); }
