#include "ParticleSystem.hpp"
#include "ReadFile.hpp"

ParticleSystem::ParticleSystem() {
    this->shaderPaths = {
        {"VertexShader", L"ParticleVS.cso"},
        {"PixelShader", L"ParticlePS.cso"},
        {"GeometryShader", L"ParticleGS.cso"},
        {"ComputeShader", L"ParticleCS.cso"},
    };
}

HRESULT ParticleSystem::Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device, UINT size, UINT nrOf, bool dynamic,
                                   bool hasSRV, bool hasUAV) {
    this->device = device;
    device->GetImmediateContext(this->immediateContext.GetAddressOf());

    HRESULT result = this->particleBuffer.Create(device, size, nrOf, dynamic, hasSRV, hasUAV);
    if (FAILED(result)) {
        return result;
    }

    return S_OK;
}

HRESULT ParticleSystem::LoadShaders() {
    for (const auto& [shaderType, filePath] : this->shaderPaths) {
        std::string byteData;
        if (!CM::ReadFile(std::string(filePath.begin(), filePath.end()), byteData)) {
            std::cerr << "Failed to read shader file: " << std::string(filePath.begin(), filePath.end()) << std::endl;
            return E_FAIL;
        }

        HRESULT result = E_FAIL;
        if (shaderType == "VertexShader") {
            result = this->device->CreateVertexShader(byteData.data(), byteData.size(), nullptr,
                                                      this->vertexShader.GetAddressOf());
        } else if (shaderType == "PixelShader") {
            result = this->device->CreatePixelShader(byteData.data(), byteData.size(), nullptr,
                                                     this->pixelShader.GetAddressOf());
        } else if (shaderType == "GeometryShader") {
            result = this->device->CreateGeometryShader(byteData.data(), byteData.size(), nullptr,
                                                        this->geometryShader.GetAddressOf());
        } else if (shaderType == "ComputeShader") {
            result = this->device->CreateComputeShader(byteData.data(), byteData.size(), nullptr,
                                                       this->computeShader.GetAddressOf());
        }

        if (FAILED(result)) {
            std::cerr << "Failed to create " << shaderType << "!\n";
            return result;
        }
    }

    if (this->vertexShader) {
        this->immediateContext->VSSetShader(this->vertexShader.Get(), nullptr, 0);
    }
    if (this->pixelShader) {
        this->immediateContext->PSSetShader(this->pixelShader.Get(), nullptr, 0);
    }
    if (this->geometryShader) {
        this->immediateContext->GSSetShader(this->geometryShader.Get(), nullptr, 0);
    }

    return S_OK;
}

ID3D11VertexShader* ParticleSystem::GetVertexShader() const { return this->vertexShader.Get(); }

ID3D11PixelShader* ParticleSystem::GetPixelShader() const { return this->pixelShader.Get(); }

ID3D11GeometryShader* ParticleSystem::GetGeometryShader() const { return this->geometryShader.Get(); }

ID3D11ShaderResourceView* ParticleSystem::GetSRV() const { return this->particleBuffer.GetSRV(); }

ID3D11UnorderedAccessView* ParticleSystem::GetUAV() const { return this->particleBuffer.GetUAV(); }

UINT ParticleSystem::GetParticleCount() const { return this->particleBuffer.GetNrOfElements(); }
