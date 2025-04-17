#include "ParticleSystem.hpp"
#include "ConstantBuffer.hpp"
#include "ReadFile.hpp"
#include <random>

ParticleSystem::ParticleSystem() : isInitialized(false) {
    this->shaderPaths = {
        {"VertexShader", "ParticleVS.cso"},
        {"GeometryShader", "ParticleGS.cso"},
        {"ComputeShader", "ParticleCS.cso"},
        {"PixelShader", "ParticlePS.cso"},
    };
}
HRESULT ParticleSystem::InitializeParticles(ID3D11Device* device, ID3D11DeviceContext* immediateContext, UINT size,
                                            UINT nrOf, bool dynamic, bool hasSRV, bool hasUAV) {
    std::vector<Particle> particles(nrOf);

    for (UINT i = 0; i < nrOf; i++) {
        particles[i].position[0] = 1;
        particles[i].position[1] = sin(i) + 10;
        particles[i].position[2] = 1;

        particles[i].velocity[0] = 1;
        particles[i].velocity[1] = 1;
        particles[i].velocity[2] = 1;

        // Varied lifetimes for natural effect
        particles[i].maxLifetime = 50;
        particles[i].lifetime    = particles[i].maxLifetime * 0.8f; // Start at 80% to create a staggered effect
    }

    HRESULT result = this->particleBuffer.Create(device, size, nrOf, dynamic, hasSRV, hasUAV, particles.data());
    if (FAILED(result)) {
        return result;
    }
    if (SUCCEEDED(result)) {
        this->isInitialized = true;
    }

    return result;
}

HRESULT ParticleSystem::LoadShaders(ID3D11Device* device, ID3D11DeviceContext* immediateContext) {
    for (const auto& [shaderType, filePath] : this->shaderPaths) {
        std::string byteData;
        if (!CM::ReadFile(filePath, byteData)) {
            std::cerr << "Failed to read shader file: " << std::string(filePath.begin(), filePath.end()) << std::endl;
            return E_FAIL;
        }

        HRESULT result = E_FAIL;
        if (shaderType == "PixelShader") {
            result =
                device->CreatePixelShader(byteData.data(), byteData.size(), nullptr, this->pixelShader.GetAddressOf());
        } else if (shaderType == "VertexShader") {
            result = device->CreateVertexShader(byteData.data(), byteData.size(), nullptr,
                                                this->vertexShader.GetAddressOf());
        } else if (shaderType == "GeometryShader") {
            result = device->CreateGeometryShader(byteData.data(), byteData.size(), nullptr,
                                                  this->geometryShader.GetAddressOf());
        } else if (shaderType == "ComputeShader") {
            result = device->CreateComputeShader(byteData.data(), byteData.size(), nullptr,
                                                 this->computeShader.GetAddressOf());
        }

        if (FAILED(result)) {
            std::cerr << "Failed to create " << shaderType << "! Error: " << result << "\n";
            return result;
        }
    }

    return S_OK;
}

void ParticleSystem::UpdateParticles(ID3D11Device* device, ID3D11DeviceContext* immediateContext, float deltaTime) {
    if (!this->isInitialized || !this->computeShader) {
        return;
    }

    // Create and update time buffer
    struct TimeBufferData {
        float deltaTime;
        float padding[3];
    } timeData;

    timeData = {deltaTime, {0.0f, 0.0f, 0.0f}};

    // Create constant buffer for the time
    ConstantBuffer timeBuffer;
    timeBuffer.Initialize(device, sizeof(TimeBufferData), &timeData);

    // Set compute shader
    immediateContext->CSSetShader(this->computeShader.Get(), nullptr, 0);

    // Bind time buffer to compute shader at register b2
    ID3D11Buffer* timeBufferPtr = timeBuffer.GetBuffer();
    immediateContext->CSSetConstantBuffers(2, 1, &timeBufferPtr);

    // Bind UAV to compute shader at register u1
    immediateContext->CSSetUnorderedAccessViews(1, 1, this->GetAddressOfUAV(), nullptr);

    // Calculate thread groups
    UINT nrOfParticles    = this->particleBuffer.GetNrOfElements();
    UINT nrOfThreadGroups = (nrOfParticles + 31) / 32;

    // Dispatch compute shader
    immediateContext->Dispatch(nrOfThreadGroups, 1, 1);

    // Unbind UAV
    ID3D11UnorderedAccessView* nullUAV[1] = {nullptr};
    immediateContext->CSSetUnorderedAccessViews(1, 1, nullUAV, nullptr);

    // Unbind constant buffer
    ID3D11Buffer* nullBuffer[1] = {nullptr};
    immediateContext->CSSetConstantBuffers(2, 1, nullBuffer);
}

void ParticleSystem::SetInputLayout(ID3D11DeviceContext* immediateContext) {
    immediateContext->IASetInputLayout(this->inputLayout.Get());
}

ID3D11ComputeShader* ParticleSystem::GetComputeShader() const { return this->computeShader.Get(); }

ID3D11VertexShader* ParticleSystem::GetVertexShader() const { return this->vertexShader.Get(); }

ID3D11PixelShader* ParticleSystem::GetPixelShader() const { return this->pixelShader.Get(); }

ID3D11GeometryShader* ParticleSystem::GetGeometryShader() const { return this->geometryShader.Get(); }

ID3D11ShaderResourceView* ParticleSystem::GetSRV() const { return this->particleBuffer.GetSRV(); }

ID3D11ShaderResourceView** ParticleSystem::GetAddressOfSRV() { return this->particleBuffer.GetSRVAddress(); }

ID3D11UnorderedAccessView* ParticleSystem::GetUAV() const { return this->particleBuffer.GetUAV(); }

ID3D11UnorderedAccessView** ParticleSystem::GetAddressOfUAV() { return this->particleBuffer.GetUAVAddress(); }

UINT ParticleSystem::GetParticleCount() const { return this->particleBuffer.GetNrOfElements(); }
