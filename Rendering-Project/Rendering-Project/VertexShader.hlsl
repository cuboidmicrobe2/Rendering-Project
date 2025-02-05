struct VertexShaderInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
    float2 uv : UV;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = float4(input.position, 1.0f);
    output.normal = float4(input.normal, 1.0f);
    output.uv = input.uv;
    
    return output;
}