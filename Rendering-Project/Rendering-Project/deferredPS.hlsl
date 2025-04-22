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
    float parallaxHeightScale;
    float3 padding;
}
cbuffer CameraData : register(b1)
{
    float4x4 _viewProj;
    float3 camPos;
    float _padding;
}

static const int parallaxSteps = 16;
static const float layerDepth = 1 / (float)parallaxSteps;
static const float defaultAmbient = 0.05;
PixelShaderOutput main(PixelShaderInput input)
{
    float3 normal;
    float2 samplePoint = input.uv;
    if (hasNormal)
    {
        // compute tangent and Bitangent
        float3 dPdx = ddx(input.worldPosition);
        float3 dPdy = ddy(input.worldPosition);
        float2 dUVdx = ddx(input.uv);
        float2 dUVdy = ddy(input.uv);

        float3 tangent = normalize(dUVdy.y * dPdx - dUVdx.y * dPdy);
        tangent = normalize(tangent - input.normal.xyz * dot(input.normal.xyz, tangent));
        
        float3 bitangent = cross(input.normal.xyz, tangent);
        
        // Convert view direction to tangent space
        float3 viewDir = normalize(input.worldPosition.xyz - camPos);
        float3x3 TBN = float3x3(tangent, bitangent, input.normal.xyz);
        float3 viewDirTangent = mul(TBN, viewDir);
        
        
        //PixelShaderOutput output;
        //output.diffuse = float4(0, 0, 0, 0);
        //output.normal = float4(normal, 0);
        //output.position = input.worldPosition;
        //output.ambient = float4(viewDir, 0);
        //output.specular = float4(0, 0, 0, 0);

        //return output;
        
        // Parallax
        //float layerDepth = 1.0 / parallaxSteps;
        float2 uvOffset = float2(0, 0);
        float2 uvOffsetPrev = uvOffset;
        float currentDepth = 0.0;

        // Delta UV per layer (constant)
        float2 deltaUV = -viewDirTangent.xy * (parallaxHeightScale / viewDirTangent.z) * layerDepth;

        float heightSample;
        for (int i = 0; i < parallaxSteps; ++i)
        {
            uvOffsetPrev = uvOffset;

             // advance into the next layer
            currentDepth += layerDepth;
            uvOffset += deltaUV;

            // sample the REAL height
            heightSample = normalMap.Sample(samplerState, input.uv + uvOffset).w;

            // if we've passed the geometry surface
            if (currentDepth >= heightSample)
            {
                // back up one step and lerp for accuracy
                float prevDepth = currentDepth - layerDepth;
                float t = (heightSample - prevDepth) / layerDepth;
                uvOffset = lerp(uvOffsetPrev, uvOffset, t);
                break;
            }
        }

        // final UV to sample N and other maps
        float2 samplePoint = input.uv + uvOffset;
                
        float3 nSample = normalMap.Sample(samplerState, samplePoint).rgb * 2 - 1;
    
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
    output.diffuse = hasDiffuse ? diffuseTexture.Sample(samplerState, samplePoint) : float4(1, 1, 1, 1);
    output.normal = float4(normal, 0);
    output.position = input.worldPosition;
    output.ambient = hasAmbient ? ambientTexture.Sample(samplerState, samplePoint) : hasDiffuse ? output.diffuse * defaultAmbient : float4(defaultAmbient, defaultAmbient, defaultAmbient, 1);
    output.specular = hasSpecular ? specularTexture.Sample(samplerState, samplePoint) : float4(1, 1, 1, 1);

    return output;
}