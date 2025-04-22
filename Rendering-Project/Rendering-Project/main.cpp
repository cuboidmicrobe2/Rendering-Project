#define NOMINMAX
#include "DCEM.hpp"
#include "DirectionalLight.hpp"
#include "Mesh.hpp"
#include "Renderer.hpp"
#include "SimpleObject.hpp"
#include "SimpleVertex.hpp"
#include "WindowHandler.hpp"
#include "testScene.hpp"
#include "BaseScene.hpp"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    Window window(hInstance, nCmdShow);
    Renderer renderer;
    HRESULT result = renderer.Init(window);
    if (FAILED(result)) {
        return result;
    }

    TestScene scene(window, renderer.GetDevice(), renderer.GetDeviceContext(), renderer.meshHandler, renderer.GetPS(), renderer.GetDCEMPS());

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        window.inputHandler.reset();

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        scene.UpdateScene();
        renderer.Render(&scene);
    }

    return 0;
}