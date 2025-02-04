#pragma once

#include <d3d11_4.h>

#include <vector>

class DepthBuffer
{
private:
	ID3D11Texture2D* texture = nullptr;
	std::vector<ID3D11DepthStencilView*> depthStencilViews;
	ID3D11ShaderResourceView* srv = nullptr;

public:
	DepthBuffer() = default;
	DepthBuffer(ID3D11Device* device, UINT width, UINT height, bool hasSRV = false);
	~DepthBuffer();
	DepthBuffer(const DepthBuffer& other) = delete;
	DepthBuffer& operator=(const DepthBuffer& other) = delete;
	DepthBuffer(DepthBuffer&& other) = delete;
	DepthBuffer& operator=(DepthBuffer&& other) = delete;

	void Initialize(ID3D11Device* device, UINT width, UINT height,
		bool hasSRV = false, UINT arraySize = 1);

	ID3D11DepthStencilView* GetDSV(UINT arrayIndex) const;
	ID3D11ShaderResourceView* GetSRV() const;
};