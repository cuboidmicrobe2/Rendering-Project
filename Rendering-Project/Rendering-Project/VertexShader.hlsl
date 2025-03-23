cbuffer ViewAndProjectionMatrix : register(b0)
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 viewProjectionMatrix;
};

cbuffer WorldMatrix : register(b1)
{
    float4x4 world;
};


struct VertexShaderInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float4 worldPosition : WORLD_POSITION;
    float4 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.worldPosition = mul(float4(input.position, 1.0f), world);
    output.position = mul(output.worldPosition, viewProjectionMatrix);
    output.normal = normalize(float4(mul(float4(input.normal, 1.0f), world).xyz, 0));
    output.texCoord = input.uv;
    return output;
}