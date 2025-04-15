#ifndef PARTICLE_BUFFER_HPP
#define PARTICLE_BUFFER_HPP
#include <d3d11.h>
#include <wrl/client.h>

class ParticleBuffer {
  public:
    ParticleBuffer(Microsoft::WRL::ComPtr<ID3D11Device> device);
    ~ParticleBuffer() = default;

  private:
    HRESULT Create();

    UINT size;
    UINT nrOf;
    bool dynamic;
    bool hasSRV;
    bool hasUAV;
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> uav;
};

#endif