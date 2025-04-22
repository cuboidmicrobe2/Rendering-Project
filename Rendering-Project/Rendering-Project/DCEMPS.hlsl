Texture2D diffuseTexture : register(t0);
Texture2D ambientTexture : register(t1);
Texture2D specularTexture : register(t2);
Texture2D normalMap : register(t3);
TextureCube shaderTexture : register(t4);
SamplerState samplerState : register(s0);

// Camera buffer (same as vertex shader)
cbuffer CameraBuffer : register(b1)
{
    float4x4 _waste;
    float3 cameraPosition;
};

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float4 worldPosition : WORLD_POSITION;
    float4 normal : NORMAL;
    float2 uv : UV;
};

cbuffer MetaData : register(b0)
{
    int hasDiffuse;
    int hasAmbient;
    int hasSpecular;
    int hasNormal;
}

struct PixelShaderOutput
{
    float4 position : SV_Target0;
    float4 diffuse : SV_Target1;
    float4 normal : SV_Target2;
    float4 ambient : SV_Target3;
    float4 specular : SV_Target4;
};


PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
    
    float3 viewDir = normalize(input.worldPosition.xyz - cameraPosition.xyz);
    float3 samplevec = normalize(reflect(viewDir, normalize(input.normal.xyz)));

    output.ambient = shaderTexture.Sample(samplerState, samplevec);
    output.normal = input.normal;
    output.position = input.worldPosition;
    output.specular = output.specular = hasSpecular ? specularTexture.Sample(samplerState, input.uv) : float4(0, 0, 0, 1);
    output.diffuse = hasDiffuse ? diffuseTexture.Sample(samplerState, input.uv) : float4(0, 0, 0, 1);

    return output;
}