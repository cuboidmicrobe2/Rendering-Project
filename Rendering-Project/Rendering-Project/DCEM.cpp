#include "DCEM.hpp"
#include "ConstantBuffer.hpp"

DCEM::DCEM(Transform transform, UINT width, UINT height, ID3D11PixelShader* normalPS, ID3D11PixelShader* DCEMPS,
           Mesh* mesh)
    : cameras({
          Camera(90, 16.f / 9.f, 1, 1000, transform.GetPosition(), {1, 0, 0}, nullptr,
                 D3D11_VIEWPORT{
                     .TopLeftX = 0,
                     .TopLeftY = 0,
                     .Width    = static_cast<FLOAT>(width),
                     .Height   = static_cast<FLOAT>(height),
                     .MinDepth = 0.0f,
                     .MaxDepth = 1.0f,
                 }),
          Camera(90, 16.f / 9.f, 1, 1000, transform.GetPosition(), {-1, 0, 0}, nullptr,
                 D3D11_VIEWPORT{
                     .TopLeftX = 0,
                     .TopLeftY = 0,
                     .Width    = static_cast<FLOAT>(width),
                     .Height   = static_cast<FLOAT>(height),
                     .MinDepth = 0.0f,
                     .MaxDepth = 1.0f,
                 }),
          Camera(90, 16.f / 9.f, 1, 1000, transform.GetPosition(), {0, 1, 0}, nullptr,
                 D3D11_VIEWPORT{
                     .TopLeftX = 0,
                     .TopLeftY = 0,
                     .Width    = static_cast<FLOAT>(width),
                     .Height   = static_cast<FLOAT>(height),
                     .MinDepth = 0.0f,
                     .MaxDepth = 1.0f,
                 }),
          Camera(90, 16.f / 9.f, 1, 1000, transform.GetPosition(), {0, -1, 0}, nullptr,
                 D3D11_VIEWPORT{
                     .TopLeftX = 0,
                     .TopLeftY = 0,
                     .Width    = static_cast<FLOAT>(width),
                     .Height   = static_cast<FLOAT>(height),
                     .MinDepth = 0.0f,
                     .MaxDepth = 1.0f,
                 }),
          Camera(90, 16.f / 9.f, 1, 1000, transform.GetPosition(), {0, 0, 1}, nullptr,
                 D3D11_VIEWPORT{
                     .TopLeftX = 0,
                     .TopLeftY = 0,
                     .Width    = static_cast<FLOAT>(width),
                     .Height   = static_cast<FLOAT>(height),
                     .MinDepth = 0.0f,
                     .MaxDepth = 1.0f,
                 }),
          Camera(90, 16.f / 9.f, 1, 1000, transform.GetPosition(), {0, 0, -1}, nullptr,
                 D3D11_VIEWPORT{
                     .TopLeftX = 0,
                     .TopLeftY = 0,
                     .Width    = static_cast<FLOAT>(width),
                     .Height   = static_cast<FLOAT>(height),
                     .MinDepth = 0.0f,
                     .MaxDepth = 1.0f,
                 }),
      }),
      SceneObject(transform), mesh(mesh) {}

HRESULT DCEM::Init(ID3D11Device* device) {
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width              = this->cameras[0].GetViewPort().Width;
    desc.Height = this->cameras[0].GetViewPort().Height;
    ;
    desc.MipLevels          = 1;
    desc.ArraySize          = 6;
    desc.Format             = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage             = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags      = D3D11_RESOURCE_MISC_TEXTURECUBE;

    device->CreateTexture2D(&desc, nullptr, this->texture.GetAddressOf());
    device->CreateShaderResourceView(this->texture.Get(), nullptr, &this->srv);

    CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Format                   = DXGI_FORMAT_B8G8R8A8_UNORM;
    uavDesc.ViewDimension            = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
    uavDesc.Texture2DArray.ArraySize = 1;
    uavDesc.Texture2DArray.MipSlice  = 0;
    for (int i = 0; i < 6; i++) {
        uavDesc.Texture2DArray.FirstArraySlice = i;
        HRESULT r = device->CreateUnorderedAccessView(this->texture.Get(), &uavDesc, this->cameras[i].GetAdressOfUAV());
        if (FAILED(r)) {
            std::cerr << "UAV Creation failed " << __LINE__ << __FILE__ << " Error: " << r << "\n";
            return r;
        }
    }
    return S_OK;
}

void DCEM::Draw(ID3D11Device* device, ID3D11DeviceContext* context) const {
    context->PSSetShader(this->PS, nullptr, 0);

    ConstantBuffer buffer;
    DirectX::XMFLOAT4X4 worldMatrix = this->GetWorldMatrix();
    buffer.Initialize(device, sizeof(worldMatrix), &worldMatrix);
    context->VSSetConstantBuffers(1, 1, buffer.GetAdressOfBuffer());

    // Bind verticies to VertexShader
    this->mesh->BindMeshBuffers(context);

    context->PSSetShaderResources(1, 1, &this->srv);

    // Draw all submeshes
    for (size_t i = 0; i < this->mesh->GetNrOfSubMeshes(); i++) {
        this->mesh->PerformSubMeshDrawCall(context, i);
    }

    context->PSSetShader(this->normalPS, nullptr, 0);
}

void DCEM::Update() {}

std::array<Camera, 6>& DCEM::GetCameras() { return this->cameras; }
