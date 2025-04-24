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
#include "CubeAndLightScene.hpp"

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

        CoolScene coolScene(window, renderer.GetDevice(), renderer.GetDeviceContext(), renderer.meshHandler,
                            renderer.GetPS(), renderer.GetDCEMPS());
        CubeMappingAndLighting scene(window, renderer.GetDevice(), renderer.GetDeviceContext(), renderer.meshHandler,
                        renderer.GetPS(), renderer.GetDCEMPS());

        BaseScene* activeScene = &scene;
        MSG msg                = {};
        std::array<BaseScene*, 3> scenes{&scene, &coolScene};

        auto lastTime = std::chrono::high_resolution_clock::now();
        while (msg.message != WM_QUIT) {
            auto currentTime                                         = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float, std::ratio<1, 1>> deltaTime = (currentTime - lastTime);
            float deltTime_f                                         = deltaTime.count();
            lastTime                                                 = currentTime;
            window.inputHandler.reset();

            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            for (int i = 0; i < std::min(9, (int) scenes.size()); i++) {
                if (window.inputHandler.wasPressed('1' + i)) {
                    std::cout << '1' + i << "\n";
                    activeScene = scenes[i];
                }
            }
            activeScene->UpdateScene(deltTime_f);
            renderer.Render(activeScene, deltTime_f);
        }
    }
    ID3D11Debug* debug = nullptr;
    if (SUCCEEDED(device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debug)))) {
        debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL); // D3D11_RLDO_IGNORE_INTERNAL
        debug->Release();
    }
    return 0;
}