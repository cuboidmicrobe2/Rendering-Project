#define NOMINMAX
#include "Mesh.hpp"
#include "Scene.hpp"
#include "SimpleVertex.hpp"
#include "WindowHandler.hpp"
#include "Renderer.hpp"
#include "SimpleObject.hpp"
#include "DCEM.hpp"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    Window window(hInstance, nCmdShow);
    Renderer renderer;
    renderer.Init(window);

    Scene scene(window);
    Mesh* mesh = scene.LoadMesh(".", "boat.obj", renderer.GetDevice());
    Mesh* mesh2 = scene.LoadMesh(".", "icoSphere.obj", renderer.GetDevice());
    SimpleObject some(Transform({0, 0, 0, 0}, DirectX::XMQuaternionIdentity(), {1, 1, 1}), mesh);
    scene.AddSceneObject(&some);

    DCEM dcem(Transform({-20, 0, 0}), static_cast<UINT>(256), static_cast<UINT>(256), renderer.GetPS(),
              renderer.GetDCEMPS(), mesh2);
    if (FAILED(dcem.Init(renderer.GetDevice()))) return -1;
    scene.AddSceneObject(&dcem);

    for (const Camera& cam : dcem.GetCameras())
        scene.AddCameraObject(cam);

    Light light(Transform({10, 0, -10}), {1, 1, 1}, 1);
    scene.AddLightObject(light);

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