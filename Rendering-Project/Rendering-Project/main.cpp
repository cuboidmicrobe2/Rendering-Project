#define NOMINMAX
#include "DCEM.hpp"
#include "DirectionalLight.hpp"
#include "Mesh.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "SimpleObject.hpp"
#include "SimpleVertex.hpp"
#include "WindowHandler.hpp"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    Window window(hInstance, nCmdShow);
    Renderer renderer;
    HRESULT result = renderer.Init(window);
    if (FAILED(result)) {
        return result;
    }

    Scene scene(window);

    // Mesh* mesh  = scene.LoadMesh(".", "boat.obj", renderer.GetDevice());
    scene.CreateObject("boat.obj", {0, 0, 30, 1}, renderer.GetDevice());

    Mesh* mesh2 = scene.LoadMesh(".", "icoSphere.obj", renderer.GetDevice());

    DCEM dcem(Transform({0, 0, 0}), renderer.GetPS(), renderer.GetDCEMPS(), mesh2);
    if (FAILED(dcem.Init(renderer.GetDevice(), 256))) return -1;
    scene.AddSceneObject(&dcem);

    for (const Camera& cam : dcem.GetCameras())
        scene.AddCameraObject(cam);

    // SimpleObject some(Transform({0, 0, 0, 0}, DirectX::XMQuaternionIdentity(), {1, 1, 1}), mesh);
    // some.InitBuffer(renderer.GetDevice());
    // scene.AddSceneObject(&some);

    // std::array<SimpleObject, 6> arr{SimpleObject(DirectX::XMVECTOR{20, 0, 0}, mesh),
    //                                 SimpleObject(DirectX::XMVECTOR{10, 0, 0}, mesh),
    //                                 SimpleObject(Transform(DirectX::XMVECTOR{0, -4, 0}, {1, 0, 0}), mesh),
    //                                 SimpleObject(DirectX::XMVECTOR{0, 10, 0}, mesh),
    //                                 SimpleObject(DirectX::XMVECTOR{0, 0, 10}, mesh),
    //                                 SimpleObject(DirectX::XMVECTOR{0, 0, -10}, mesh)};
    // for (auto& o : arr) {

    //    o.InitBuffer(renderer.GetDevice());
    //    scene.AddSceneObject(&o);
    //}
    Light light(Transform({0, 4, 0}, 0, 90), {1, 1, 1}, 5, 45);
    scene.AddLightObject(light);

    DirectionalLight dirLight(Transform({0, 0, 0}, 90 + 45, 0), {1, 1, 1}, 1, 100, 100);
    scene.AddDirLight(dirLight);

    if (FAILED(scene.Init(renderer.GetDevice(), renderer.GetDeviceContext()))) return -1;

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        window.inputHandler.reset();

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        scene.UpdateScene();
        renderer.Render(scene);
    }

    return 0;
}