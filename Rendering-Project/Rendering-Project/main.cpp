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
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    {
        Window window(hInstance, nCmdShow);
        Renderer renderer;
        HRESULT result = renderer.Init(window);
        if (FAILED(result)) {
            return result;
        }
        device         = renderer.GetDeviceCOMPTR();

        TestScene scene(window, renderer.GetDevice(), renderer.GetDeviceContext(), renderer.meshHandler,
                        renderer.GetPS(), renderer.GetDCEMPS());

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
    }
    ID3D11Debug* debug = nullptr;
    if (SUCCEEDED(device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debug)))) {
        debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL); // D3D11_RLDO_IGNORE_INTERNAL
        debug->Release();
    }
    return 0;
}