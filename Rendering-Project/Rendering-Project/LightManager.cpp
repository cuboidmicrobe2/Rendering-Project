#include "LightManager.hpp"

LightManager::LightManager(UINT resoulution) : resoulution(resoulution), viewport(D3D11_VIEWPORT{
                                    .TopLeftX = 0,
                                    .TopLeftY = 0,
                                    .Width    = static_cast<FLOAT>(resoulution),
                                    .Height   = static_cast<FLOAT>(resoulution),
                                    .MinDepth = 0.0f,
                                    .MaxDepth = 1.0f,
                                }) {}

HRESULT LightManager::Init(ID3D11Device* device) {
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width              = this->resoulution;
    desc.Height             = this->resoulution;
    desc.MipLevels          = 1;
    desc.ArraySize          = this->Spotlights.size();
    desc.Format             = DXGI_FORMAT_R24G8_TYPELESS;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage              = D3D11_USAGE_DEFAULT;
    desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
    desc.CPUAccessFlags     = 0;
    desc.MiscFlags          = 0;

    HRESULT r = device->CreateTexture2D(&desc, nullptr, this->depthStencils.GetAddressOf());
    if (FAILED(r)) {
        std::cerr << "Creating texture Failed, Error: " << r << "\n";
        return r;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format                         = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension                  = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2DArray.MostDetailedMip = 0;
    srvDesc.Texture2DArray.MipLevels       = 1;
    srvDesc.Texture2DArray.FirstArraySlice = 0;
    srvDesc.Texture2DArray.ArraySize       = this->Spotlights.size();

    r = device->CreateShaderResourceView(this->depthStencils.Get(), &srvDesc, &this->shaderResourceView);
    if (FAILED(r)) {
        std::cerr << "Creating srv Failed, Error: " << r << "\n";
        return r;
    }
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format                   = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension            = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
    dsvDesc.Texture2DArray.ArraySize = 1;
    dsvDesc.Texture2DArray.MipSlice  = 0;

    for (int i = 0; i < this->Spotlights.size(); i++) {
        dsvDesc.Texture2DArray.FirstArraySlice = i;
        HRESULT r = this->Spotlights[i].Init(device, this->resoulution, &dsvDesc, this->depthStencils.Get());
        if (FAILED(r)) {
            std::cerr << "Creating dsv Failed, Error: " << r << "\n";
            return r;
        }
    }
    std::vector<LightData> data = this->GetRawLightData();
    this->lightBuffer.Initialize(device, sizeof(LightData), data.size(), data.data());
}

std::vector<LightData> LightManager::GetRawLightData() const {
    std::vector<LightData> lightData;
    lightData.reserve(this->Spotlights.size());
    for (int i = 0; i < this->Spotlights.size(); i++) {
        float* tempPos   = this->Spotlights[i].transform.GetPosition().m128_f32;
        float* tempColor = this->Spotlights[i].GetColor().m128_f32;
        float* tempDir   = this->Spotlights[i].GetDirection().m128_f32;
        // Create and Bind view and projection matrixes
        DirectX::XMMATRIX viewMatrix       = this->Spotlights[i].CreateViewMatrix();
        DirectX::XMMATRIX projectionMatrix = this->Spotlights[i].CreateProjectionMatrix();

        LightData l{
            .pos{tempPos[0], tempPos[1], tempPos[2]},
            .intensity = this->Spotlights[i].GetIntesity(),
            .color     = {tempColor[0], tempColor[1], tempColor[2], tempColor[3]},
            .direction = {tempDir[0], tempDir[1], tempDir[2]},
            .angle     = cos(this->Spotlights[i].GetAngle() / 2),
        };
        DirectX::XMStoreFloat4x4(&l.viewProjectionMatrix, DirectX::XMMatrixMultiplyTranspose(viewMatrix, projectionMatrix));
        lightData.emplace_back(l);
    }
    return lightData;
}

ID3D11ShaderResourceView* const* LightManager::GetAdressOfDSSRV() const {
    return this->shaderResourceView.GetAddressOf();
}

const std::vector<Light>& LightManager::GetLights() const { return this->Spotlights; }

void LightManager::AddLight(const Light& light) { this->Spotlights.emplace_back(light); }

const D3D11_VIEWPORT& LightManager::GetViewPort() const { return this->viewport; }

void LightManager::BindLightData(ID3D11DeviceContext* context, UINT slot) { 
    context->CSSetShaderResources(slot, 1, this->lightBuffer.GetAdressOfSRV()); 
}

void LightManager::BindDepthTextures(ID3D11DeviceContext* context, UINT slot) {
    context->CSSetShaderResources(slot, this->Spotlights.size(), this->GetAdressOfDSSRV());
}

void LightManager::UnbindDepthTextures(ID3D11DeviceContext* context, UINT slot) {
    ID3D11ShaderResourceView* nullsrvs[3]{};
    context->CSSetShaderResources(slot, this->Spotlights.size(), nullsrvs);
}
