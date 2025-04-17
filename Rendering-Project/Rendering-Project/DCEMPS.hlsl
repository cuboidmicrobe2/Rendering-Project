TextureCube shaderTexture : register(t1);
SamplerState samplerState : register(s0);

cbuffer camPos : register(b1)
{
    float4 pos;
};

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
    
    float4 vec = input.position - pos;
    
    float4 samplevec = reflect(vec, input.normal);

    output.colour = shaderTexture.Sample(samplerState, vec.xyz);
    output.normal = input.normal;
    output.position = input.worldPosition;

    return output;
}