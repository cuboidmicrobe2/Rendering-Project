#define NOMINMAX
#include "Mesh.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
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
    if (FAILED(scene.Init(renderer.GetDevice(), renderer.GetDeviceContext()))) return -1;
    Mesh* mesh = scene.LoadMesh(".", "boat.obj", renderer.GetDevice());

    SceneObject some(Transform({0, 0, 0, 0}, DirectX::XMQuaternionIdentity(), {1, 1, 1}), mesh);
    some.InitBuffer(renderer.GetDevice());
    scene.AddSceneObject(&some);

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