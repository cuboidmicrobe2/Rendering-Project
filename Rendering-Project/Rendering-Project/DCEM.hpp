#include "Camera.hpp"
#include "SceneObject.hpp"
#include "Transform.hpp"
#include <d3d11.h>

class DCEM : public SceneObject {
  public:
    DCEM(Transform transform, UINT width, UINT height, ID3D11PixelShader* normalPS, ID3D11PixelShader* DCEMPS,
         Mesh* mesh);
    HRESULT Init(ID3D11Device* device);
    void Draw(ID3D11Device* device, ID3D11DeviceContext* context) const override;
    void Update() override;

  private:
    ID3D11Texture2D* texture;
    Mesh* mesh;
    ID3D11PixelShader* PS;
    ID3D11PixelShader* normalPS;
    std::array<Camera, 6> cameras;
    ID3D11ShaderResourceView* srv;
    ID3D11RenderTargetView* rtv;
};