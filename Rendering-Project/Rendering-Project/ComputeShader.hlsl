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

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4 pixelPosition = float4(positionGBuffer[DTid.xy].xyz, 0);
    float4 normal = float4(normalize(normalGBuffer[DTid.xy].xyz), 0);
    float4 CamToPixel = pixelPosition - float4(cameraPos, 0);
    
    float diffuse = 0;
    float specular = 0;
    for (int i = 0; i < nrofLights; i++)
    {
        Light cl = lights[i];
        float4 lightClip = mul(float4(pixelPosition.xyz, 1), cl.vpMatrix);
        float3 ndc = lightClip.xyz / lightClip.w;
        
        float2 uv = float2(ndc.x * 0.5f + 0.5f, ndc.y * -0.5f + 0.5f);
        
        float sceneDepth = ndc.z;
        float mapDepth = shadowMaps.SampleLevel(shadowSampler, float3(uv, i), 0.f).r;
        
        const float bias = 0.001f;
        bool lit = (mapDepth + bias) >= sceneDepth;
        
        
        float4 LightToHit = pixelPosition - float4(cl.pos, 0);
        float4 lightDir = normalize(LightToHit);
        if (dot(lightDir.xyz, normalize(cl.direction)) > cl.cosAngle && lit)
        {
            float intensity = (1 / dot(LightToHit, LightToHit)) * max(0.0f, dot(-lightDir, normal));
            diffuse += intensity;
    
            float4 halfWayVector = normalize(lightDir + normalize(CamToPixel));
            float specularDot = max(dot(normal, -halfWayVector), 0);
            specular += pow(specularDot, 100);            
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
        bool lit = (mapDepth + bias) >= sceneDepth;
        
        if (lit)
        {
            float4 lightDir = float4(normalize(cl.direction), 0);
            float intensity =  max(0.0f, dot(-lightDir, normal));
            diffuse += intensity;
    
            float4 halfWayVector = normalize(lightDir + normalize(CamToPixel));
            float specularDot = max(dot(normal, -halfWayVector), 0);
            specular += pow(specularDot, 100);
        }
    }
    float4 ambientComponent = ambientGBuffer[DTid.xy];
    float4 diffuseComponent = diffuseGBuffer[DTid.xy] * diffuse;
    float4 specularComponent = specularGBuffer[DTid.xy] * specular;
    backBufferUAV[float3(DTid.xy, 0)] = ambientComponent + diffuseComponent + specularComponent;
} 