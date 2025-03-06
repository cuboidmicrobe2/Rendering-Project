#define NOMINMAX
#include "Mesh.hpp"
#include "Scene.hpp"
#include "SimpleVertex.hpp"
#include "WindowHandler.hpp"
#include <WICTextureLoader.h>
#include <chrono>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    Window window(hInstance, nCmdShow);

    Scene scene(window);
    Mesh* mesh = scene.LoadMesh("source/Cube.obj");
    SceneObject some(Transform({0, 0, 0, 0}, DirectX::XMQuaternionIdentity(), {1, 1, 1}), mesh);
    scene.AddSceneObject(some);

    Camera camera(90, 16.f / 9.f, 1, 1000, {0, 0, -10}, {0, 0, 1});
    scene.AddCameraObject(camera);

    Light light(Transform({1, 0, -1}), {1, 1, 1}, 1);
    scene.AddLightObject(light);

    // tempstuff end

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        window.inputHandler.reset();

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        scene.UpdateScene();
        scene.RenderScene();
    }

    return 0;
}