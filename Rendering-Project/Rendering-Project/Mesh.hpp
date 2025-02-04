#pragma once

#include <vector>

#include <d3d11_4.h>

#include "SubMesh.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"

struct MeshData
{
	struct VertexInfo
	{
		size_t sizeOfVertex;
		size_t nrOfVerticesInBuffer;
		void* vertexData;
	} vertexInfo;

	struct IndexInfo
	{
		size_t nrOfIndicesInBuffer;
		uint32_t* indexData;
	} indexInfo;

	struct SubMeshInfo
	{
		size_t startIndexValue;
		size_t nrOfIndicesInSubMesh;
		ID3D11ShaderResourceView* ambientTextureSRV;
		ID3D11ShaderResourceView* diffuseTextureSRV;
		ID3D11ShaderResourceView* specularTextureSRV;
	};

	std::vector<SubMeshInfo> subMeshInfo;
};

class Mesh
{
private:
	std::vector<SubMesh> subMeshes;
	VertexBuffer vertexBuffer;
	IndexBuffer indexBuffer;

public:
	Mesh() = default;
	~Mesh() = default;
	Mesh(const Mesh & other) = delete;
	Mesh& operator=(const Mesh & other) = delete;
	Mesh(Mesh && other) = delete;
	Mesh& operator=(Mesh && other) = delete;

	void Initialize(ID3D11Device* device, const MeshData& meshInfo);

	void BindMeshBuffers(ID3D11DeviceContext* context) const;
	void PerformSubMeshDrawCall(ID3D11DeviceContext* context, size_t subMeshIndex) const;

	size_t GetNrOfSubMeshes() const;
	ID3D11ShaderResourceView* GetAmbientSRV(size_t subMeshIndex) const;
	ID3D11ShaderResourceView* GetDiffuseSRV(size_t subMeshIndex) const;
	ID3D11ShaderResourceView* GetSpecularSRV(size_t subMeshIndex) const;
};