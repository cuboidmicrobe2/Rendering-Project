Texture2D shaderTexture : register(t0);


cbuffer MetaData : register(b0)
{
    int nrOfLights;
};

cbuffer lights : register(b1)
{
    // This might not be fully completly safe
    struct lightStruct
    {
        float4 lightPosition;
        float4 lightDirectionQuaternion;
        float4 scale;
        
        float4 alignment;
        
        float4 color;
        float lightIntesity;
    } lights[1];
};

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
    float2 uv : UV;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    return float4(input.uv, 1.0f, 1.0f);
}