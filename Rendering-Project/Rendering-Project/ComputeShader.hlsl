RWTexture2D<unorm float4> backBufferUAV : register(u0);
Texture2D<float4> positionGBuffer : register(t0);
Texture2D<float4> colorGBuffer : register(t1);
Texture2D<float4> normalGBuffer : register(t2);
// Some lights and potentially other resources
// ...

struct Light
{
    float3 pos;
    float intensity;
    float4 color;
};

cbuffer lightbuffer : register(b1)
{
    Light lights[128];
};

cbuffer metadata : register(b0)
{
    int nrofLights;
    float3 cameraPos;
    
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float3 position = positionGBuffer[DTid.xy].xyz;
    float3 color = colorGBuffer[DTid.xy].xyz;
    float4 normal = float4(normalize(normalGBuffer[DTid.xy].xyz), 0);
    
    float4 result;
    for (int i = 0; i < nrofLights; i++)
    {
        Light cl = lights[i];
        
        float4 ambientComponent = cl.color * 0.05;
    
        float4 lightDirection = float4(normalize(cl.pos - position), 0);
        float intensity = max(0, dot(normal, lightDirection)) * cl.intensity;
        float4 diffuseComponent = float4(color * intensity, 1);
    
        float4 reflection = reflect(-lightDirection, normal);
        float4 pixelToCamera = float4(normalize(cameraPos - position), 0);
        float specular = pow(max(0, dot(reflection, pixelToCamera)), 1);
        float4 specularComponent = float4(color * specular, 1);
        
        result += specularComponent + (ambientComponent + diffuseComponent) * colorGBuffer[DTid.xy];
    }
    result /= nrofLights;
    backBufferUAV[DTid.xy] = result;
    
}