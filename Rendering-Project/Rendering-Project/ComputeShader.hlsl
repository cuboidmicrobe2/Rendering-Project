RWTexture2DArray<unorm float4> backBufferUAV : register(u0);
SamplerState shadowSampler : register(s0);
Texture2D<float4> positionGBuffer : register(t0);
Texture2D<float4> diffuseGBuffer : register(t1);
Texture2D<float4> normalGBuffer : register(t2);
Texture2D<float4> ambientGBuffer : register(t3);
Texture2D<float4> specularGBuffer : register(t4);
Texture2DArray<unorm float> shadowMaps : register(t5);
Texture2DArray<unorm float> DirShadowMaps : register(t6);
// Some lights and potentially other resources
// ...

cbuffer RenderMode : register(b10)
{
    int renderingMode;
    float3 _padding;
}

struct Light
{
    float3 pos;
    float _padding;
    float4 color;
    float3 direction;
    float cosAngle;
    float4x4 vpMatrix;
};

StructuredBuffer<Light> lights : register(t7);
StructuredBuffer<Light> DirLights : register(t8);

cbuffer metadata : register(b0)
{
    int nrofLights;
    int nrofDirLights;
    float2 padding;
    float3 cameraPos;
    float alsoPadding;
}

static const int renderingModeStandard = 0;
static const int renderingModeDiffuse = 1;
static const int renderingModeNormals = 2;
static const int renderingModeAmbient = 3;
static const int renderingModeSpecular = 4;
static const int renderingModePosition = 5;

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (renderingMode == renderingModeStandard)
    {
    
        float4 pixelPosition = float4(positionGBuffer[DTid.xy].xyz, 0);
        float4 normal = float4(normalize(normalGBuffer[DTid.xy].xyz), 0);
        float4 CamToPixel = pixelPosition - float4(cameraPos, 0);
    
        float4 diffuse = 0;
        float4 specular = 0;
        float specularExponent = specularGBuffer[DTid.xy].w;
    // Spotlights
        for (int i = 0; i < nrofLights; i++)
        {
            Light cl = lights[i];
            float4 lightClip = mul(float4(pixelPosition.xyz, 1), cl.vpMatrix);
            float3 ndc = lightClip.xyz / lightClip.w;
        
            float2 uv = float2(ndc.x * 0.5f + 0.5f, ndc.y * -0.5f + 0.5f);
        
            float sceneDepth = ndc.z;
            float mapDepth = shadowMaps.SampleLevel(shadowSampler, float3(uv, i), 0.f).r;
        
            const float bias = 0.000001f;
            bool islit = (mapDepth + bias) >= sceneDepth;
        
        
            float4 LightToHit = pixelPosition - float4(cl.pos, 0);
            float4 lightDir = normalize(LightToHit);
            if (dot(lightDir.xyz, normalize(cl.direction)) > cl.cosAngle && islit)
            {
                float intensity = (1 / dot(LightToHit, LightToHit)) * max(0.0f, dot(-lightDir, normal));
    
                float4 halfWayVector = normalize(lightDir + normalize(CamToPixel));
                float specularDot = max(dot(normal, -halfWayVector), 0);
                float4 lighting = lit(intensity, specularDot, specularExponent);
            
                diffuse += lighting.y * cl.color;
                specular += lighting.z * cl.color;
            }
        }
    
    // Dir Lights
        for (int i = 0; i < nrofDirLights; i++)
        {
            Light cl = DirLights[i];
            float4 lightClip = mul(float4(pixelPosition.xyz, 1), cl.vpMatrix);
            float3 ndc = lightClip.xyz / lightClip.w;
        
            float2 uv = float2(ndc.x * 0.5f + 0.5f, ndc.y * -0.5f + 0.5f);
        
            float sceneDepth = ndc.z;
            float mapDepth = DirShadowMaps.SampleLevel(shadowSampler, float3(uv, i), 0.f).r;
        
            const float bias = 0.005f;
            bool isLit = (mapDepth + bias) >= sceneDepth || mapDepth == 1;
        
            if (isLit)
            {
                float4 lightDir = float4(normalize(cl.direction), 0);
                float diffuseIntensity = max(0.0f, dot(-lightDir, normal));
    
                float4 halfWayVector = normalize(lightDir + normalize(CamToPixel));
                float specularDot = max(dot(normal, -halfWayVector), 0);
                float4 lighting = lit(diffuseIntensity, specularDot, specularExponent);
            
                diffuse += lighting.y * cl.color;
                specular += lighting.z * cl.color;
            }
        }
        float4 ambientComponent = ambientGBuffer[DTid.xy];
        float4 diffuseComponent = diffuseGBuffer[DTid.xy] * diffuse;
        float4 specularComponent = specularGBuffer[DTid.xy] * specular;
        backBufferUAV[float3(DTid.xy, 0)] = ambientComponent + diffuseComponent + specularComponent;
    }
    else if (renderingMode == renderingModeAmbient)
    {
        backBufferUAV[float3(DTid.xy, 0)] = ambientGBuffer[DTid.xy];
    }
    else if (renderingMode == renderingModeDiffuse)
    {
        backBufferUAV[float3(DTid.xy, 0)] = diffuseGBuffer[DTid.xy];
    }
    else if (renderingMode == renderingModeNormals)
    {
        backBufferUAV[float3(DTid.xy, 0)] = normalGBuffer[DTid.xy];
    }
    else if (renderingMode == renderingModeSpecular)
    {
        backBufferUAV[float3(DTid.xy, 0)] = specularGBuffer[DTid.xy];
    }
    else if (renderingMode == renderingModePosition)
    {
        backBufferUAV[float3(DTid.xy, 0)] = normalize(positionGBuffer[DTid.xy]);
    }
} 