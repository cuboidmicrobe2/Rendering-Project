#ifndef SCENE_HPP
#define SCENE_HPP

#include "Camera.hpp"
#include "InputHandler.hpp"
#include "WindowHandler.hpp"
#include "Light.hpp"
#include "ParticleBuffer.hpp"
#include "SceneObject.hpp"
#include <memory>
#include <vector>

class Scene {
  public:
    Scene(Window& window);
    ~Scene();

    void AddSceneObject(const SceneObject& sceneObject);
    void AddCameraObject(const Camera& camera);
    void AddLightObject(const Light& light);

    std::vector<Camera>& getCameras();
    const std::vector<Light>& getLights();
    const std::vector<SceneObject>& getObjects();

    void RenderParticles(ParticleBuffer& particleBuffer, ID3D11VertexShader* vertexShader,
                         ID3D11GeometryShader* geometryShader, ID3D11PixelShader* pixelShader,
                         ID3D11ShaderResourceView* smokeTexture);
    void UpdateParticles(ParticleBuffer& particleBuffer, ID3D11ComputeShader*& computeShader);
    Mesh* LoadMesh(const std::filesystem::path& folder, const std::string& objname, ID3D11Device* device);

    Camera& getMainCam();

    void RenderScene();
    void UpdateScene();

  private:
    Camera mainCamera;
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