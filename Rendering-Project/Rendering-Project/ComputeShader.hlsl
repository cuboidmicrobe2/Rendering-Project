RWTexture2D<unorm float4> backBufferUAV : register(u0);

Texture2D<float4> positionGBuffer : register(t0);
Texture2D<float4> colorGBuffer : register(t1);
Texture2D<float4> normalGBuffer : register(t2);
// Some lights and potentially other resources
// ...
[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float3 position = positionGBuffer[DTid.xy].xyz;
    float3 colour = colorGBuffer[DTid.xy].xyz;
    float3 normal = normalize(normalGBuffer[DTid.xy].xyz);
    // Perform lighting calculations
    // ...
    backBufferUAV[DTid.xy] = float4(colour.xyz, 1.0f);
}