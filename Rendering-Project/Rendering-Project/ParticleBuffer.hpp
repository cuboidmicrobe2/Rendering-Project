#ifndef PARTICLE_BUFFER_HPP
#define PARTICLE_BUFFER_HPP
#include <d3d11.h>
#include <wrl/client.h>

class ParticleBuffer {
  public:
    ParticleBuffer();
    ~ParticleBuffer() = default;

    UINT GetNrOfElements() const;
    ID3D11Buffer* GetBuffer() const;
    ID3D11ShaderResourceView* GetSRV() const;
    ID3D11ShaderResourceView** GetSRVAddress();
    ID3D11UnorderedAccessView* GetUAV() const;
    ID3D11UnorderedAccessView** GetUAVAddress();

    HRESULT Create(ID3D11Device* device, UINT size, UINT nrOf, bool dynamic, bool hasSRV, bool hasUAV, void* initData);

  private:
    UINT size;
    UINT nrOf;
    bool dynamic;
    Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> uav;
};

#endif