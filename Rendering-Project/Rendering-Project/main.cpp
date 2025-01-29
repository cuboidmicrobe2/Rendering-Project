#include "WindowHandler.h"

// Main : ) 
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {

	Window window(hInstance, 1920, 1080, nCmdShow);

	MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
            DispatchMessage(&msg);
		}
	}

	return 0;
}

