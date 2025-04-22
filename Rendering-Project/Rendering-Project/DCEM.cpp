#include "DCEM.hpp"
#include "ConstantBuffer.hpp"

inline DirectX::XMVECTOR LookRotation(DirectX::XMVECTOR forward, DirectX::XMVECTOR up) {
    using namespace DirectX;

    XMVECTOR z = XMVector3Normalize(forward);
    XMVECTOR x = XMVector3Normalize(XMVector3Cross(up, z));
    XMVECTOR y = XMVector3Cross(z, x);

    XMMATRIX rotationMatrix = {x.m128_f32[0], x.m128_f32[1], x.m128_f32[2], 0.0f,          y.m128_f32[0], y.m128_f32[1],
                               y.m128_f32[2], 0.0f,          z.m128_f32[0], z.m128_f32[1], z.m128_f32[2], 0.0f,
                               0.0f,          0.0f,          0.0f,          1.0f};

    return XMQuaternionRotationMatrix(rotationMatrix);
}

DCEM::DCEM(Transform transform, ID3D11PixelShader* normalPS, ID3D11PixelShader* DCEMPS, Mesh* mesh, UINT size)
    : cameras({
          Camera(90, 1, 1, 1000, transform.GetPosition(), LookRotation({1, 0, 0}, {0, 1, 0}), nullptr, &this->rr),
          Camera(90, 1, 1, 1000, transform.GetPosition(), LookRotation({-1, 0, 0}, {0, 1, 0}), nullptr, &this->rr),
          Camera(90, 1, 1, 1000, transform.GetPosition(), LookRotation({0, 1, 0}, {0, 0, -1}), nullptr, &this->rr),
          Camera(90, 1, 1, 1000, transform.GetPosition(), LookRotation({0, -1, 0}, {0, 0, 1}), nullptr, &this->rr),
          Camera(90, 1, 1, 1000, transform.GetPosition(), LookRotation({0, 0, 1}, {0, 1, 0}), nullptr, &this->rr),
          Camera(90, 1, 1, 1000, transform.GetPosition(), LookRotation({0, 0, -1}, {0, 1, 0}), nullptr, &this->rr),
      }),
      SceneObject(transform, mesh), PS(DCEMPS), normalPS(normalPS), srv(nullptr), size(size) {}

void DCEM::Init(ID3D11Device* device) {
    DirectX::XMFLOAT4X4 matrix = this->GetWorldMatrix();
    this->matrixBuffer.Initialize(device, sizeof(matrix), &matrix);

    this->rr.Init(device, size, size);

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width              = size;
    desc.Height             = size;
    desc.MipLevels          = 1;
    desc.ArraySize          = 6;
    desc.Format             = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage              = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    desc.CPUAccessFlags     = 0;
    desc.MiscFlags          = D3D11_RESOURCE_MISC_TEXTURECUBE;

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
            throw std::runtime_error("Failed to create DCEM");
        }
    }
}

void DCEM::Draw(ID3D11Device* device, ID3D11DeviceContext* context) {
    context->PSSetShader(this->PS, nullptr, 0);

    DirectX::XMFLOAT4X4 matrix = this->GetWorldMatrix();
    this->matrixBuffer.UpdateBuffer(context, &matrix);
    context->VSSetConstantBuffers(1, 1, this->matrixBuffer.GetAdressOfBuffer());

    // Bind verticies to VertexShader
    this->mesh->BindMeshBuffers(context);

    context->PSSetShaderResources(4, 1, &this->srv);

    // Draw all submeshes
    for (size_t i = 0; i < this->mesh->GetNrOfSubMeshes(); i++) {
        this->mesh->PerformSubMeshDrawCall(context, i);
    }

    context->PSSetShader(this->normalPS, nullptr, 0);
}

void DCEM::Update() {}

std::array<Camera, 6>& DCEM::GetCameras() { return this->cameras; }
