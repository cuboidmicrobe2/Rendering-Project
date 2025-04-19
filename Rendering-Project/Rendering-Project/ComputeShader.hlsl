RWTexture2DArray<unorm float4> backBufferUAV : register(u0);
SamplerState shadowSampler : register(s0);
Texture2D<float4> positionGBuffer : register(t0);
Texture2D<float4> colorGBuffer : register(t1);
Texture2D<float4> normalGBuffer : register(t2);
Texture2DArray<unorm float> shadowMaps : register(t3);
// Some lights and potentially other resources
// ...

struct Light
{
    float3 pos;
    float intensity;
    float4 color;
    float3 direction;
    float cosAngle;
    float4x4 vpMatrix;
};

StructuredBuffer<Light> lights : register(t4);

cbuffer metadata : register(b0)
{
    int nrofLights;
    float3 cameraPos;
    
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4 position = float4(positionGBuffer[DTid.xy].xyz, 0);
    float4 color = float4(colorGBuffer[DTid.xy].xyz, 1);
    float4 normal = float4(normalize(normalGBuffer[DTid.xy].xyz), 0);
    
    float4 result = float4(0, 0, 0, 0);
    float4 ambientComponent = 0.1;
    for (int i = 0; i < nrofLights; i++)
    {
        Light cl = lights[i];
        float4 lightClip = mul(float4(position.xyz, 1), cl.vpMatrix);
        float3 ndc = lightClip.xyz / lightClip.w;
        
        float2 uv = float2(ndc.x * 0.5f + 0.5f, ndc.y * -0.5f + 0.5f);
        
        float sceneDepth = ndc.z;
        float mapDepth = shadowMaps.SampleLevel(shadowSampler, float3(uv, i), 0.f).r;
        
        const float bias = 0.001f;
        bool lit = (mapDepth + bias) >= sceneDepth;
        
        float4 hitToLight = float4(cl.pos, 0) - position;
        float4 lightDirection = normalize(hitToLight);
        
        if (dot(-lightDirection.xyz, normalize(cl.direction)) > cl.cosAngle && lit)
        {
            float intensity = 1 / dot(hitToLight, hitToLight) * cl.intensity;
            float4 diffuseComponent = color * intensity;
    
            float4 reflection = reflect(-lightDirection, normal);
            float4 pixelToCamera = normalize(float4(cameraPos, 0) - position);
            float specular = pow(max(0, dot(reflection, pixelToCamera)), 100);
            float4 specularComponent = color * specular;
        
            result += specularComponent + diffuseComponent * colorGBuffer[DTid.xy];
        }
    }
    result = result + colorGBuffer[DTid.xy] * ambientComponent;
    backBufferUAV[float3(DTid.xy, 0)] = result;
}