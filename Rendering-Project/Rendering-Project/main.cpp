#define NOMINMAX
#include "BaseScene.hpp"
#include "CoolScene.hpp"
#include "DCEM.hpp"
#include "DirectionalLight.hpp"
#include "Mesh.hpp"
#include "Renderer.hpp"
#include "SimpleObject.hpp"
#include "SimpleVertex.hpp"
#include "TestScene2.hpp"
#include "WindowHandler.hpp"
#include "testScene.hpp"

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
        device = renderer.GetDeviceCOMPTR();

        TestScene scene(window, renderer.GetDevice(), renderer.GetDeviceContext(), renderer.meshHandler,
                        renderer.GetPS(), renderer.GetDCEMPS());
        TestScene2 scene2(window, renderer.GetDevice(), renderer.GetDeviceContext(), renderer.meshHandler,
                          renderer.GetPS(), renderer.GetDCEMPS());
        CoolScene coolScene(window, renderer.GetDevice(), renderer.GetDeviceContext(), renderer.meshHandler,
                            renderer.GetPS(), renderer.GetDCEMPS());

        BaseScene* activeScene = &scene;
        MSG msg                = {};
        while (msg.message != WM_QUIT) {
            window.inputHandler.reset();

            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            if (window.inputHandler.wasPressed('1')) {
                activeScene = &scene;
            }
            if (window.inputHandler.wasPressed('2')) {
                activeScene = &scene2;
            }
            if (window.inputHandler.wasPressed('3')) {
                activeScene = &coolScene;
            }
            activeScene->UpdateScene();
            renderer.Render(activeScene);
        }
    }
    ID3D11Debug* debug = nullptr;
    if (SUCCEEDED(device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debug)))) {
        debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL); // D3D11_RLDO_IGNORE_INTERNAL
        debug->Release();
    }
    return 0;
}