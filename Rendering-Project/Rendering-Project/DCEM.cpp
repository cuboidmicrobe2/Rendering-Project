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

HRESULT DCEM::Init(ID3D11Device* device) { return E_NOTIMPL; }

void DCEM::Draw(ID3D11Device* device, ID3D11DeviceContext* context) const {
    context->PSSetShader(this->PS, nullptr, 0);

    ConstantBuffer buffer;
    DirectX::XMFLOAT4X4 worldMatrix = this->GetWorldMatrix();
    buffer.Initialize(device, sizeof(worldMatrix), &worldMatrix);
    context->VSSetConstantBuffers(1, 1, buffer.GetAdressOfBuffer());

    // Bind verticies to VertexShader
    this->mesh->BindMeshBuffers(context);

    // Draw all submeshes
    for (size_t i = 0; i < this->mesh->GetNrOfSubMeshes(); i++) {
        this->mesh->PerformSubMeshDrawCall(context, i);
    }

    context->PSSetShader(this->normalPS, nullptr, 0);
}

void DCEM::Update() {}
