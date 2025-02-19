#include "Mesh.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "SimpleVertex.hpp"
#include "WindowHandler.hpp"
#include <chrono>
#include <WICTextureLoader.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool CreateTexture(ID3D11Device* device, ID3D11Texture2D*& texture, ID3D11ShaderResourceView*& srv,
                   unsigned char*& imageData) {
    int height, width, channels;
    imageData = stbi_load("slimepfp.jpg", &width, &height, &channels, 0);
    assert(channels == 3 && "nrof channels was not 3");

    int newChannels            = channels + 1;
    unsigned char* textureData = new unsigned char[height * width * newChannels];

    for (int i = 0; i < height * width; i++) {
        for (int j = 0; j < channels; j++)
            textureData[i * newChannels + j] = imageData[i * channels + j];
        textureData[i * newChannels + channels] = 255;
    }

    delete[] imageData;
    imageData = textureData;

    D3D11_TEXTURE2D_DESC textureDesc;
    textureDesc.Width          = width;
    textureDesc.Height         = height;
    textureDesc.MipLevels      = 1;
    textureDesc.ArraySize      = 1;
    textureDesc.Format         = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc     = DXGI_SAMPLE_DESC{1, 0};
    textureDesc.Usage          = D3D11_USAGE_IMMUTABLE;
    textureDesc.BindFlags      = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags      = 0;

    D3D11_SUBRESOURCE_DATA textureSubData;
    textureSubData.pSysMem          = imageData;
    textureSubData.SysMemPitch      = width * newChannels;
    textureSubData.SysMemSlicePitch = 0;

    HRESULT createShaderResult = device->CreateTexture2D(&textureDesc, &textureSubData, &texture);

    if (FAILED(createShaderResult)) {
        std::cerr << "Failed to create 2D texture\n";
        return false;
    }

    HRESULT createSRVResult = device->CreateShaderResourceView(texture, nullptr, &srv);
    return !FAILED(createSRVResult);
}


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


    ID3D11Texture2D* texture;
    ID3D11ShaderResourceView* srv;

    //HRESULT createShaderResult =
    //    DirectX::CreateWICTextureFromFile(renderer.GetDevice(), L"slimepfp.jpg", &texture, &srv);
    unsigned char* imageData;
    CreateTexture(renderer.GetDevice(), texture, srv, imageData);

    assert(texture, "texture was null");

 

    renderer.GetDevice()->CreateShaderResourceView(texture, nullptr, &srv);

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
        scene.RenderScene(renderer.GetDevice(), renderer.GetContext());
        renderer.Present();
    }

    delete[] imageData;
    return 0;
}