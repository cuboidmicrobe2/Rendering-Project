TextureCube shaderTexture : register(t1);
SamplerState samplerState : register(s0);

// Camera buffer (same as vertex shader)
cbuffer CameraBuffer : register(b1)
{
    float4 cameraPosition;
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
    
    float3 viewDir = normalize(input.worldPosition.xyz - cameraPosition.xyz);
    float3 samplevec = normalize(reflect(viewDir, normalize(input.normal.xyz)));

    output.colour = shaderTexture.Sample(samplerState, samplevec);
    //output.colour = float4(normalize(cameraPosition.xyz), 1);
    output.normal = input.normal;
    output.position = input.worldPosition;

    return output;
}