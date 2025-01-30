#include "WindowHandler.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {

	const UINT WIDTH = 1920;
    const UINT HEIGHT = 1080;

	Window window(hInstance, WIDTH, HEIGHT, nCmdShow);

	MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
            DispatchMessage(&msg);
		}
	}

	return 0;
}