#ifndef MESH_HANDLER
#define MESH_HANDLER
#include <string>
#include "Mesh.hpp"
#include <unordered_map>
#include <d3d11_4.h>
class MeshHandler {
  public:
    MeshHandler() {}
    ~MeshHandler() {}
    Mesh* GetMesh(const std::string& folder, const std::string& objname, ID3D11Device* device) {
        std::string key = folder + "/" + objname;
        auto it         = this->meshes.find(key);
        if (it != this->meshes.end())
            return it->second.get();
        else {
            this->meshes.emplace(key, std::make_unique<Mesh>(device, folder, objname));
            return this->meshes.at(key).get();
        }
    }

  private:
    std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes;
};

#endif