#pragma once

#include <d3d11_4.h>

class ConstantBuffer
{
private:
	ID3D11Buffer* buffer = nullptr;
	size_t bufferSize = 0;

public:
	ConstantBuffer() = default;
	ConstantBuffer(ID3D11Device* device, size_t byteSize, void* initialData = nullptr);
	~ConstantBuffer();
	ConstantBuffer(const ConstantBuffer& other) = delete;
	ConstantBuffer& operator=(const ConstantBuffer& other) = delete;
	ConstantBuffer(ConstantBuffer&& other) noexcept; // Move constructor
	ConstantBuffer& operator=(ConstantBuffer&& other) noexcept; // Move assignment operator

	void Initialize(ID3D11Device* device, size_t byteSize, void* initialData = nullptr);

	size_t GetSize() const;
	ID3D11Buffer* GetBuffer() const;
    ID3D11Buffer** GetAdressOfBuffer();

	void UpdateBuffer(ID3D11DeviceContext* context, void* data);
};