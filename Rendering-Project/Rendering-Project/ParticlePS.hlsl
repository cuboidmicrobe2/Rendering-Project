// ParticlePS.hlsl

// Texture sampler for smoke pattern
Texture2D smokeTexture : register(t0);
SamplerState textureSampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR0;
};

float4 main(PSInput input) : SV_TARGET
{
    // Sample the base smoke texture
    float4 smokePattern = smokeTexture.Sample(textureSampler, input.texCoord);
    
    // Create turbulence effect by distorting UV coordinates
    float2 distortedUV = input.texCoord + float2(
        sin(input.texCoord.y * 10.0f) * 0.02f,
        cos(input.texCoord.x * 10.0f) * 0.02f
    );
    
    // Sample with distorted coordinates for additional detail
    float4 distortedPattern = smokeTexture.Sample(textureSampler, distortedUV);
    
    // Combine both samples for more interesting pattern
    float alphaMask = smokePattern.r * distortedPattern.g;
    
    // Apply a soft falloff near edges
    alphaMask *= smoothstep(0.0, 0.4, 1.0 - length(input.texCoord - 0.5) * 1.4);
    
    // Create the final smoke color
    // Base is the passed color (which is based on lifetime)
    float4 finalColor = input.color;
    
    // Apply some subtle color variation based on the texture
    finalColor.rgb = lerp(
        finalColor.rgb,
        finalColor.rgb * (0.7 + 0.3 * distortedPattern.b),
        0.5
    );
    
    // Set alpha based on texture and lifetime
    finalColor.a *= alphaMask;
    
    return finalColor;
}
