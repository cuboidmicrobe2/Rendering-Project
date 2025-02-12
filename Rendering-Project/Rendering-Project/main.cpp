#include "Mesh.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "SimpleVertex.hpp"
#include "WindowHandler.hpp"
#include <chrono>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    Window window(hInstance, nCmdShow);

    Renderer renderer(window);

    if (FAILED(renderer.Init())) {
        return -1;
    }

    // Temp stuff
    Mesh mesh;
    SimpleVertex vertices[] = {
        {{-0.5f, 0.5f, 0.0f}, {0, 0, -1}, {0, 0}},
        {{0.5f, 0.5f, 0.0f}, {0, 0, -1}, {1, 0}},
        {{-0.5f, -0.5f, 0.0f}, {0, 0, -1}, {0, 1}},
        {{0.5f, -0.5f, 0.0f}, {0, 0, -1}, {1, 1}},
    };
    unsigned indices[6]{1, 2, 3, 3, 2, 4};
    mesh.Initialize(renderer.GetDevice(), MeshData{MeshData::VertexInfo{sizeof(SimpleVertex), 4, vertices},
                                                   MeshData::IndexInfo{6, indices},
                                                   {MeshData::SubMeshInfo{0, 6, nullptr, nullptr, nullptr}}});

    Scene scene;

    SceneObject obj({0, 0, 0}, mesh);
    scene.addSceneObject(obj);

    Camera camera(90, 16.f / 9.f, 0, 1000, {0, 0, -1}, {0, 0, 1});
    scene.addCameraObject(camera);

    // tempstuff end

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        window.inputHandler.reset();

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        renderer.Update();
    }

    return 0;
}