#pragma once

#include <d3d11_4.h>

class StructuredBuffer
{
private:
	ID3D11Buffer* buffer = nullptr;
	ID3D11ShaderResourceView* srv = nullptr;
	UINT elementSize = 0;
	size_t nrOfElements = 0;

public:
	StructuredBuffer() = default;
	StructuredBuffer(ID3D11Device* device, UINT sizeOfElement,
		size_t nrOfElementsInBuffer, void* bufferData = nullptr, bool dynamic = true);
	~StructuredBuffer();
	StructuredBuffer(const StructuredBuffer& other) = delete;
	StructuredBuffer& operator=(const StructuredBuffer& other) = delete;
	StructuredBuffer(StructuredBuffer&& other) = delete;
	StructuredBuffer operator=(StructuredBuffer&& other) = delete;

	void Initialize(ID3D11Device* device, UINT sizeOfElement,
		size_t nrOfElementsInBuffer, void* bufferData = nullptr, bool dynamic = true);

	void UpdateBuffer(ID3D11DeviceContext* context, void* data);

	UINT GetElementSize() const;
	size_t GetNrOfElements() const;
	ID3D11ShaderResourceView* GetSRV() const;
	ID3D11ShaderResourceView** GetAdressOfSRV();
};