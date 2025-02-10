struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
    float2 uv : UV;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    return float4(input.uv, 1.0f, 1.0f);
}