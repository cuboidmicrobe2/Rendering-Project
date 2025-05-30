struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float4 worldPosition : WORLD_POSITION;
    float4 normal : NORMAL;
    float2 uv : UV;
    
};

cbuffer DistanceToObject : register(b0)
{
    float distance;
}

struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[3] : SV_TessFactor;
    float InsideTessFactor : SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VertexShaderOutput, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
    HS_CONSTANT_DATA_OUTPUT Output;
    float baseTess = 15.0f;
    float minTess = 0.01f;
    float maxDistance = 50.0f;
    
    float tessFactor = lerp(baseTess, minTess, saturate(distance / maxDistance));
    
    Output.EdgeTessFactor[0] = Output.EdgeTessFactor[1] = Output.EdgeTessFactor[2] = Output.InsideTessFactor = tessFactor;

    return Output;
}

struct HullShaderOutput
{
    float3 worldPosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
HullShaderOutput main(
	InputPatch<VertexShaderOutput, NUM_CONTROL_POINTS> ip,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID)
{
    HullShaderOutput output;

    output.worldPosition = ip[i].worldPosition.xyz;
    output.normal = ip[i].normal.xyz;
    output.uv = ip[i].uv;
    
    

    return output;
}
