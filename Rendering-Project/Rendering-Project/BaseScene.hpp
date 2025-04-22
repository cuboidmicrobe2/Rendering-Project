#include "WindowHandler.hpp"
#include "SceneObject.hpp"
#include "SimpleObject.hpp"
#include "Camera.hpp"
#include "Light.hpp"
#include "DirectionalLight.hpp"
#include "LightManager.hpp"
#include "ParticleSystem.hpp"

class BaseScene {
  public:

	BaseScene(Window& window);
    ~BaseScene();

    void AddSceneObject(SceneObject* sceneObject);
    void AddBoundingBox(SceneObject* box);

    void CreateObject(Mesh* mesh, const DirectX::XMVECTOR& position, ID3D11Device* device,
                      const std::string& folder = ".");

    void AddCameraObject(const Camera& camera);
    void AddLightObject(const Light& light);
    void AddDirLight(const DirectionalLight& light);
    LightManager& GetLightManager();

    HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* immediateContext);

    std::vector<Camera>& getCameras();
    const std::vector<Light>& getLights();
    std::vector<SceneObject*>& GetBoundingBoxes();
    std::vector<SceneObject*>& GetVisibleObjects();

    ParticleSystem& GetParticleSystem();

    Camera& getMainCam();

    void UpdateScene();
};