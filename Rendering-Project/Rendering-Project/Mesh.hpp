#ifndef MESH_HPP
#define MESH_HPP
#define DIRECTXTK_WIC_LOADER_FLAGS

#include <vector>

#include <d3d11_4.h>

#include "IndexBuffer.hpp"
#include "SubMesh.hpp"
#include "VertexBuffer.hpp"
#include <DirectXCollision.h>
#include <filesystem>

class Mesh {
  private:
    std::vector<SubMesh> subMeshes;
    VertexBuffer vertexBuffer;
    IndexBuffer indexBuffer;
    DirectX::BoundingBox boundingBox;

  public:
    Mesh() = default;
    Mesh(ID3D11Device* device, const std::string& folderpath, const std::string& objname);
    ~Mesh(){
    
    }
    Mesh(const Mesh& other)            = delete;
    Mesh& operator=(const Mesh& other) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other)      = delete;

    //void Initialize(ID3D11Device* device, const MeshData& meshInfo);
    void Initialize(ID3D11Device* device, const std::string& folderpath, const std::string& objname);

    void BindMeshBuffers(ID3D11DeviceContext* context) const;
    void PerformSubMeshDrawCall(ID3D11DeviceContext* context, size_t subMeshIndex);

    size_t GetNrOfSubMeshes() const;
    const std::vector<SubMesh>& GetSubMeshes() const;
    ID3D11ShaderResourceView* GetAmbientSRV(size_t subMeshIndex) const;
    ID3D11ShaderResourceView* GetDiffuseSRV(size_t subMeshIndex) const;
    ID3D11ShaderResourceView* GetSpecularSRV(size_t subMeshIndex) const;
    ID3D11ShaderResourceView* GetNormalMapSRV(size_t subMeshIndex) const;

    DirectX::BoundingBox GetBoundingBox() const;
};

#endif