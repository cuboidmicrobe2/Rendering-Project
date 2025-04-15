Texture2D shaderTexture : register(t0);
SamplerState samplerState : register(s0);

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float4 worldPosition : WORLD_POSITION;
    float4 normal : NORMAL;
    float2 uv : UV;
};

struct PixelShaderOutput
{
    float4 position : SV_Target0;
    float4 colour : SV_Target1;
    float4 normal : SV_Target2;
};


PixelShaderOutput main(PixelShaderInput input)
{

    PixelShaderOutput output;

    output.colour = shaderTexture.Sample(samplerState, input.uv);
    output.normal = input.normal;
    output.position = input.worldPosition;

    return output;
}