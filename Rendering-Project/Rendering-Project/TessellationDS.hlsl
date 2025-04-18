cbuffer Camera : register(b0)
{
    float4x4 padding[2];
    float4x4 vp;
};

struct DomainShaderOutput
{
    float4 position : SV_POSITION;
    float3 worldPosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

struct HullShaderOutput
{
    float3 worldPosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[3] : SV_TessFactor;
    float InsideTessFactor : SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

[domain("tri")]
DomainShaderOutput main(HS_CONSTANT_DATA_OUTPUT input, float3 barycentric : SV_DomainLocation,
	                    const OutputPatch<HullShaderOutput, NUM_CONTROL_POINTS> patch)
{
    DomainShaderOutput output;
    
    float3 linearPosition = patch[0].worldPosition * barycentric.x + patch[1].worldPosition * barycentric.y + patch[2].worldPosition * barycentric.z;
    float3 phongPosition = float3(0, 0, 0);
    
    for (int i = 0; i < NUM_CONTROL_POINTS; ++i)
    {
        float3 cPointToLinearPos = linearPosition - patch[i].worldPosition;
        float proj = dot(cPointToLinearPos, patch[i].normal);
        float3 pointOnTangetPlane = patch[i].worldPosition + cPointToLinearPos - proj * patch[i].normal;
        
        phongPosition += barycentric[i] * pointOnTangetPlane;
    }
    
    output.worldPosition = lerp(linearPosition, phongPosition, 0.7f);
    output.normal = normalize(patch[0].normal * barycentric.x + patch[1].normal * barycentric.y + patch[2].normal * barycentric.z);
    output.uv = patch[0].uv * barycentric.x + patch[1].uv * barycentric.y + patch[2].uv * barycentric.z;
    output.position = mul(float4(output.worldPosition, 1), vp);

    return output;
}
