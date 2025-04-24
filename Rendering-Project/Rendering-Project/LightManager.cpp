#include "LightManager.hpp"

LightManager::LightManager(UINT spotLightRes, UINT dirLightRes)
    : spotLightViewPort(D3D11_VIEWPORT{
          .TopLeftX = 0,
          .TopLeftY = 0,
          .Width    = static_cast<FLOAT>(spotLightRes),
          .Height   = static_cast<FLOAT>(spotLightRes),
          .MinDepth = 0.0f,
          .MaxDepth = 1.0f,
      }),
      directionLightViewPort(D3D11_VIEWPORT{
          .TopLeftX = 0,
          .TopLeftY = 0,
          .Width    = static_cast<FLOAT>(dirLightRes),
          .Height   = static_cast<FLOAT>(dirLightRes),
          .MinDepth = 0.0f,
          .MaxDepth = 1.0f,
      }) {}

HRESULT LightManager::Init(ID3D11Device* device) {
    if (!this->Spotlights.empty()) {
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width              = this->spotLightViewPort.Width;
        desc.Height             = this->spotLightViewPort.Height;
        desc.MipLevels          = 1;
        desc.ArraySize          = this->Spotlights.size();
        desc.Format             = DXGI_FORMAT_R24G8_TYPELESS;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = D3D11_USAGE_DEFAULT;
        desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
        desc.CPUAccessFlags     = 0;
        desc.MiscFlags          = 0;

        HRESULT r = device->CreateTexture2D(&desc, nullptr, this->SpotLightDepthTex.GetAddressOf());
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

        r = device->CreateShaderResourceView(this->SpotLightDepthTex.Get(), &srvDesc,
                                             this->SpotLightSRV.GetAddressOf());
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
            HRESULT r = this->Spotlights[i].Init(device, this->spotLightViewPort.Width, &dsvDesc,
                                                 this->SpotLightDepthTex.Get());
            if (FAILED(r)) {
                std::cerr << "Creating dsv Failed, Error: " << r << "\n";
                return r;
            }
        }
        std::vector<LightData> data = this->GetSpotLightData();
        this->spotLightBuffer.Initialize(device, sizeof(LightData), data.size(), data.data());
    }
    if (!this->directionalLights.empty()) {
        // Directional Lights
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width                = this->directionLightViewPort.Width;
        desc.Height               = this->directionLightViewPort.Height;
        desc.MipLevels            = 1;
        desc.ArraySize            = this->directionalLights.size();
        desc.Format               = DXGI_FORMAT_R24G8_TYPELESS;
        desc.SampleDesc.Count     = 1;
        desc.SampleDesc.Quality   = 0;
        desc.Usage                = D3D11_USAGE_DEFAULT;
        desc.BindFlags            = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
        desc.CPUAccessFlags       = 0;
        desc.MiscFlags            = 0;

        HRESULT r = device->CreateTexture2D(&desc, nullptr, this->dirLightDepthTex.GetAddressOf());
        if (FAILED(r)) {
            std::cerr << "Creating texture Failed, Error: " << r << "\n";
            return r;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format                          = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        srvDesc.ViewDimension                   = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        srvDesc.Texture2DArray.MostDetailedMip  = 0;
        srvDesc.Texture2DArray.MipLevels        = 1;
        srvDesc.Texture2DArray.FirstArraySlice  = 0;
        srvDesc.Texture2DArray.ArraySize        = this->directionalLights.size();

        r = device->CreateShaderResourceView(this->dirLightDepthTex.Get(), &srvDesc, this->DirLightSRV.GetAddressOf());
        if (FAILED(r)) {
            std::cerr << "Creating srv Failed, Error: " << r << "\n";
            return r;
        }

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format                        = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsvDesc.ViewDimension                 = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
        dsvDesc.Texture2DArray.ArraySize      = 1;
        dsvDesc.Texture2DArray.MipSlice       = 0;

        for (int i = 0; i < this->directionalLights.size(); i++) {
            dsvDesc.Texture2DArray.FirstArraySlice = i;
            HRESULT r = this->directionalLights[i].Init(device, this->directionLightViewPort.Width, &dsvDesc,
                                                        this->dirLightDepthTex.Get());
            if (FAILED(r)) {
                std::cerr << "Creating dsv Failed, Error: " << r << "\n";
                return r;
            }
        }
        std::vector<LightData> data = this->GetDirLightData();
        this->dirLightBuffer.Initialize(device, sizeof(LightData), data.size(), data.data());
    }
    return S_OK;
}

std::vector<LightData> LightManager::GetSpotLightData() const {
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
        DirectX::XMStoreFloat4x4(&l.viewProjectionMatrix,
                                 DirectX::XMMatrixMultiplyTranspose(viewMatrix, projectionMatrix));
        lightData.emplace_back(l);
    }
    return lightData;
}

std::vector<LightData> LightManager::GetDirLightData() const {
    std::vector<LightData> lightData;
    lightData.reserve(this->directionalLights.size());
    for (int i = 0; i < this->directionalLights.size(); i++) {
        float* tempPos   = this->directionalLights[i].transform.GetPosition().m128_f32;
        float* tempColor = this->directionalLights[i].GetColor().m128_f32;
        float* tempDir   = this->directionalLights[i].GetDirection().m128_f32;
        // Create and Bind view and projection matrixes
        DirectX::XMMATRIX viewMatrix       = this->directionalLights[i].CreateViewMatrix();
        DirectX::XMMATRIX projectionMatrix = this->directionalLights[i].CreateProjectionMatrix();
        LightData l{
            .pos{tempPos[0], tempPos[1], tempPos[2]},
            .intensity = this->directionalLights[i].GetIntesity(),
            .color     = {tempColor[0], tempColor[1], tempColor[2], tempColor[3]},
            .direction = {tempDir[0], tempDir[1], tempDir[2]},
            .angle     = 0, // unused but needed for padding
        };
        DirectX::XMStoreFloat4x4(&l.viewProjectionMatrix,
                                 DirectX::XMMatrixMultiplyTranspose(viewMatrix, projectionMatrix));
        lightData.emplace_back(l);
    }
    return lightData;
}

ID3D11ShaderResourceView* const* LightManager::GetAdressOfSpotlightDSSRV() const {
    return this->SpotLightSRV.GetAddressOf();
}

const std::vector<Light>& LightManager::GetSpotLights() const { return this->Spotlights; }

void LightManager::AddSpotLight(const Light& light) { this->Spotlights.emplace_back(light); }

ID3D11ShaderResourceView* const* LightManager::GetAdressOfDirlightDSSRV() const {
    return this->DirLightSRV.GetAddressOf();
}

void LightManager::AddDirectionalLight(const DirectionalLight& light) { this->directionalLights.emplace_back(light); }

const std::vector<DirectionalLight>& LightManager::GetDirectionalLights() const { return this->directionalLights; }

const D3D11_VIEWPORT& LightManager::GetDirectionalLightVP() const { return this->directionLightViewPort; }

const D3D11_VIEWPORT& LightManager::GetSpotLightVP() const { return this->spotLightViewPort; }

void LightManager::BindLightData(ID3D11DeviceContext* context, UINT SpotLightSlot, UINT DirLightSlot) {
    context->CSSetShaderResources(SpotLightSlot, 1, this->spotLightBuffer.GetAdressOfSRV());
    context->CSSetShaderResources(DirLightSlot, 1, this->dirLightBuffer.GetAdressOfSRV());
}

void LightManager::BindDepthTextures(ID3D11DeviceContext* context, UINT SpotLightSlot, UINT DirLightSlot) {
    context->CSSetShaderResources(SpotLightSlot, 1, this->GetAdressOfSpotlightDSSRV());
    context->CSSetShaderResources(DirLightSlot, 1, this->GetAdressOfDirlightDSSRV());
}

void LightManager::UnbindDepthTextures(ID3D11DeviceContext* context, UINT SpotLightSlot, UINT DirLightSlot) {
    ID3D11ShaderResourceView* nullsrvs[3]{};
    context->CSSetShaderResources(SpotLightSlot, 1, nullsrvs);
    context->CSSetShaderResources(DirLightSlot, 1, nullsrvs);
}
