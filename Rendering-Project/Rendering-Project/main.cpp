#include "Mesh.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "SimpleVertex.hpp"
#include "WindowHandler.hpp"
#include <WICTextureLoader.h>
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
        {{-0.5f, 0.5f, 0.0f}, {0, 0, -1}, {0, 0}},  {{0.5f, 0.5f, 0.0f}, {0, 0, -1}, {1, 0}},
        {{-0.5f, -0.5f, 0.0f}, {0, 0, -1}, {0, 1}}, {{0.5f, -0.5f, 0.0f}, {0, 0, -1}, {1, 1}},
    };

    ID3D11Resource* texture;
    ID3D11ShaderResourceView* srv;

    HRESULT createShaderResult =
        DirectX::CreateWICTextureFromFile(renderer.GetDevice(), L"slimepfp.jpg", &texture, &srv);

    assert(!FAILED(createShaderResult), "texture creation failed");

    unsigned indices[4]{0, 1, 2, 3};
    mesh.Initialize(renderer.GetDevice(), MeshData{MeshData::VertexInfo{sizeof(SimpleVertex), 4, vertices},
                                                   MeshData::IndexInfo{4, indices},
                                                   {MeshData::SubMeshInfo{0, 4, srv, srv, srv}}});

    Scene scene(window);

    SceneObject obj(Transform({0, 0, 0}), mesh);
    SceneObject ob2(Transform({0.5, 0, 0}), mesh);
    scene.AddSceneObject(obj);
    scene.AddSceneObject(ob2);

    Camera camera(90, 16.f / 9.f, 1, 1000, {0, 0, -5}, {0, 0, 1});
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
        renderer.Clear();
        scene.UpdateScene();
        scene.RenderScene(renderer.GetDevice(), renderer.GetContext());
        renderer.Present();
    }

    return 0;
}