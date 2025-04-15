#include <d3d11.h>
#include "Camera.hpp"
#include "SceneObject.hpp"
#include "Transform.hpp"


class DCEM : public SceneObject {
public:
    DCEM(Transform transform, UINT width, UINT height);
    void Draw(ID3D11Device* device, ID3D11DeviceContext* context) const override;
    void Update() override;

private:
    Camera cameras[6];
    ID3D11ShaderResourceView* srv;
    ID3D11RenderTargetView* rtv;
};