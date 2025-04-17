#include "ParticleSystem.hpp"
#include "ConstantBuffer.hpp"
#include "ReadFile.hpp"
#include <random>

ParticleSystem::ParticleSystem() : isInitialized(false) {
    this->shaderPaths = {
        {"VertexShader", L"ParticleVS.cso"},
        {"PixelShader", L"ParticlePS.cso"},
        {"GeometryShader", L"ParticleGS.cso"},
        {"ComputeShader", L"ParticleCS.cso"},
    };
}

HRESULT ParticleSystem::Initialize(ID3D11Device* device, UINT size, UINT nrOf, bool dynamic, bool hasSRV, bool hasUAV) {

    HRESULT result = this->particleBuffer.Create(device, size, nrOf, dynamic, hasSRV, hasUAV);
    if (FAILED(result)) {
        return result;
    }

    return S_OK;
}

HRESULT ParticleSystem::InitializeParticles(ID3D11DeviceContext* immediateContext, UINT count) {
    std::vector<Particle> particles(count);

    for (UINT i = 0; i < count; i++) {
        particles[i].position[0] = (float) ((rand() % 200) - 100) / 10.0f;
        particles[i].position[1] = (float) ((rand() % 200) - 100) / 10.0f;
        particles[i].position[2] = (float) ((rand() % 200) - 100) / 10.0f;

        particles[i].velocity[0] = (float) ((rand() % 200) - 100) / 10.0f;
        particles[i].velocity[1] = (float) ((rand() % 200) - 100) / 10.0f;
        particles[i].velocity[2] = (float) ((rand() % 200) - 100) / 10.0f;

        particles[i].maxLifetime = 1.0f + (float) (rand() % 9);
        particles[i].lifetime    = particles[i].maxLifetime;
    }

    HRESULT result = this->particleBuffer.Update(immediateContext, particles.data());
    if (SUCCEEDED(result)) {
        this->isInitialized = true;
    }

    return result;
}

HRESULT ParticleSystem::LoadShaders(ID3D11Device* device, ID3D11DeviceContext* immediateContext) {
    for (const auto& [shaderType, filePath] : this->shaderPaths) {
        std::string byteData;
        if (!CM::ReadFile(std::string(filePath.begin(), filePath.end()), byteData)) {
            std::cerr << "Failed to read shader file: " << std::string(filePath.begin(), filePath.end()) << std::endl;
            return E_FAIL;
        }

        HRESULT result = E_FAIL;
        if (shaderType == "VertexShader") {
            result = device->CreateVertexShader(byteData.data(), byteData.size(), nullptr,
                                                this->vertexShader.GetAddressOf());

            D3D11_INPUT_ELEMENT_DESC layoutDesc[4] = {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"LIFETIME", 0, DXGI_FORMAT_D32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"MAX_LIFETIME", 0, DXGI_FORMAT_D32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0}};

            HRESULT result = device->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc), byteData.data(),
                                                       byteData.size(), this->inputLayout.GetAddressOf());
            if (FAILED(result)) {
                return result;
            }

        } else if (shaderType == "PixelShader") {
            result =
                device->CreatePixelShader(byteData.data(), byteData.size(), nullptr, this->pixelShader.GetAddressOf());
        } else if (shaderType == "GeometryShader") {
            result = device->CreateGeometryShader(byteData.data(), byteData.size(), nullptr,
                                                  this->geometryShader.GetAddressOf());
        } else if (shaderType == "ComputeShader") {
            result = device->CreateComputeShader(byteData.data(), byteData.size(), nullptr,
                                                 this->computeShader.GetAddressOf());
        }

        if (FAILED(result)) {
            std::cerr << "Failed to create " << shaderType << "!\n";
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
