#include "Renderer.hpp"
#include "WindowHandler.hpp"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    const UINT WIDTH  = 1920;
    const UINT HEIGHT = 1080;

    Window window(hInstance, nCmdShow);

    Renderer renderer(window);

    if (FAILED(renderer.Init())) {
        return -1;
    }

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        window.inputHandler.reset();

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        renderer.Render();
    }

    return 0;
}