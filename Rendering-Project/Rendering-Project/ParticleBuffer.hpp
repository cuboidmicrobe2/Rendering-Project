#ifndef PARTICLE_BUFFER_HPP
#define PARTICLE_BUFFER_HPP
#include <d3d11.h>
#include <wrl/client.h>

class ParticleBuffer {
  public:
    ParticleBuffer(Microsoft::WRL::ComPtr<ID3D11Device> device);
    ~ParticleBuffer();

  private:
    HRESULT Create();

    UINT size;
    UINT nrOf;
    bool dynamic;
    bool hasSRV;
    bool hasUAV;
    Microsoft::WRL::ComPtr<ID3D11Device> device;
};

#endif