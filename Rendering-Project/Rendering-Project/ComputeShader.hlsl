RWTexture2DArray<unorm float4> backBufferUAV : register(u0);
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
    //backBufferUAV[DTid.xy] = float4(0, 0, 0, 0);
    //return;
    float4 position = float4(positionGBuffer[DTid.xy].xyz, 0);
    float4 color = float4(colorGBuffer[DTid.xy].xyz, 0);
    float4 normal = float4(normalize(normalGBuffer[DTid.xy].xyz), 0);
    
    float4 result;
    for (int i = 0; i < nrofLights; i++)
    {
        Light cl = lights[i];
        
        float4 ambientComponent = cl.color * 0.25;
    
        float4 lightDirection = normalize(float4(cl.pos, 0) - position);
        float intensity = max(0, dot(normal, lightDirection)) * cl.intensity;
        float4 diffuseComponent = color * intensity;
    
        float4 reflection = reflect(-lightDirection, normal);
        float4 pixelToCamera = normalize(float4(cameraPos, 0) - position);
        float specular = pow(max(0, dot(reflection, pixelToCamera)), 100);
        float4 specularComponent = color * specular;
        
        result += specularComponent + (ambientComponent + diffuseComponent) * colorGBuffer[DTid.xy];
    }
    result /= nrofLights;
    backBufferUAV[float3(DTid.xy, 0)] = result;
}