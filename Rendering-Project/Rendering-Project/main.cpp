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

    Mesh* mesh = scene.LoadMesh(".", "boat.obj", renderer.GetDevice());
    for (int i = 0; i < 1; i++) {
        float x = (i % 5) * 20.0f - 40.0f;
        float y = ((i % 3) - 1) * 5.0f;
        float z = (i / 5) * 20.0f + 10.0f;
        scene.CreateObject(mesh, {x, y, z, 1}, renderer.GetDevice());
    }

    Mesh* mesh2 = scene.LoadMesh(".", "icoSphere.obj", renderer.GetDevice());

    DCEM dcem(Transform({0, 0, 0}), renderer.GetPS(), renderer.GetDCEMPS(), mesh2);
    if (FAILED(dcem.Init(renderer.GetDevice(), 256))) return -1;
    scene.AddSceneObject(&dcem);

    for (const Camera& cam : dcem.GetCameras())
        scene.AddCameraObject(cam);

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