#pragma once

#include <d3d11_4.h>

class VertexBuffer
{
private:
	ID3D11Buffer* buffer = nullptr;
	UINT nrOfVertices = 0;
	UINT vertexSize = 0;

public:
	VertexBuffer() = default;
	VertexBuffer(ID3D11Device* device, UINT sizeOfVertex, 
		UINT nrOfVerticesInBuffer, void* vertexData);
	~VertexBuffer();
	VertexBuffer(const VertexBuffer& other) = delete;
	VertexBuffer& operator=(const VertexBuffer& other) = delete;
	VertexBuffer(VertexBuffer&& other) = delete;
	VertexBuffer& operator=(VertexBuffer&& other) = delete;

	void Initialize(ID3D11Device* device, UINT sizeOfVertex,
		UINT nrOfVerticesInBuffer, void* vertexData);

	UINT GetNrOfVertices() const;
	UINT GetVertexSize() const;
	ID3D11Buffer* GetBuffer() const;
};