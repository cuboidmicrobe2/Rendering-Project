Texture2D diffuseTexture : register(t0);
Texture2D ambientTexture : register(t1);
Texture2D specularTexture : register(t2);
Texture2D normalMap : register(t3);
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
    float4 diffuse : SV_Target1;
    float4 normal : SV_Target2;
    float4 ambient : SV_Target3;
    float4 specular : SV_Target4;
};

cbuffer MetaData : register(b0)
{
    int hasDiffuse;
    int hasAmbient;
    int hasSpecular;
    int hasNormal;
    int hasParallax;
    int padding[3];
}

static const float defaultAmbient = 0.05;
PixelShaderOutput main(PixelShaderInput input) 
{
    float3 normal;
    if (hasNormal)
    {
        float3 dPdx = ddx(input.worldPosition);
        float3 dPdy = ddy(input.worldPosition);
        float2 dUVdx = ddx(input.uv);
        float2 dUVdy = ddy(input.uv);

        float3 tangent = normalize(dUVdy.y * dPdx - dUVdx.y * dPdy);
        float3 bitangent = normalize(-dUVdy.x * dPdx + dUVdx.x * dPdy);

        tangent = normalize(tangent - input.normal.xyz * dot(input.normal.xyz, tangent));
        bitangent = cross(input.normal.xyz, tangent);
        
        float3 nSample = normalMap.Sample(samplerState, input.uv).rgb * 2 - 1;
    
        float3 worldNormal = normalize(
            nSample.x * tangent +
            nSample.y * bitangent +
            nSample.z * input.normal.xyz
        );
        normal = worldNormal.xyz;
    }
    else
    {
        normal = input.normal;
    }

    PixelShaderOutput output;
    output.diffuse = hasDiffuse ? diffuseTexture.Sample(samplerState, input.uv) : float4(1, 1, 1, 1);
    output.normal = float4(normal, 0);
    output.position = input.worldPosition;
    output.ambient = hasAmbient ? ambientTexture.Sample(samplerState, input.uv) : hasDiffuse ? output.diffuse * defaultAmbient : float4(defaultAmbient, defaultAmbient, defaultAmbient, 1);
    output.specular = hasSpecular ? specularTexture.Sample(samplerState, input.uv) : float4(1, 1, 1, 1);

    return output;
}