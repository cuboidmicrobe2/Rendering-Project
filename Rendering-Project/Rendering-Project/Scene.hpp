#ifndef SCENE_HPP
#define SCENE_HPP

#include "Camera.hpp"
#include "DeferredRenderer.hpp"
#include "ForwardRenderer.hpp"
#include "InputHandler.hpp"
#include "Light.hpp"
#include "ParticleBuffer.hpp"
#include "SceneObject.hpp"
#include <memory>
#include <vector>

class Scene {
  public:
    Scene(Window& window);
    ~Scene();
    HRESULT Init();

    void AddSceneObject(const SceneObject& sceneObject);
    void AddCameraObject(const Camera& camera);
    void AddLightObject(const Light& light);

    Mesh* LoadMesh(const std::filesystem::path& folder, const std::string& objname);

    void RenderParticles(ParticleBuffer& particleBuffer, ID3D11VertexShader* vertexShader,
                         ID3D11GeometryShader* geometryShader, ID3D11PixelShader* pixelShader,
                         ID3D11ShaderResourceView* smokeTexture);
    void UpdateParticles(ParticleBuffer& particleBuffer, ID3D11ComputeShader*& computeShader);

    void RenderScene();
    void UpdateScene();

  private:
    std::unique_ptr<DaddyRenderer> renderer;
    InputHandler& input;

    std::vector<std::unique_ptr<Mesh>> meshes;
    std::vector<SceneObject> objects;
    std::vector<Camera> cameras;
    std::vector<Light> lights;

    ParticleBuffer particleBuffer;
    ID3D11VertexShader* vertexShader;
    ID3D11GeometryShader* geometryShader;
    ID3D11PixelShader* pixelShader;
    ID3D11ComputeShader* computeShader;
    ID3D11ShaderResourceView* srv;
};

#endif