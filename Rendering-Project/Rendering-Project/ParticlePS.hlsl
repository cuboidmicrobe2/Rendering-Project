struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR0;
};

float4 main(PSInput input) : SV_TARGET
{
    return input.color;
    // Create a procedural pattern using only UV coordinates
    float2 uv = input.texCoord;
    
    // Create turbulence effect with UV coordinates
    float2 distortedUV = uv + float2(
        sin(uv.y * 10.0f) * 0.02f,
        cos(uv.x * 10.0f) * 0.02f
    );
    
    // Generate procedural patterns
    float pattern1 = (sin(uv.x * 20.0f) + 1.0f) * 0.5f;
    float pattern2 = (cos(distortedUV.y * 15.0f) + 1.0f) * 0.5f;
    
    // Combine patterns for more interesting results
    float alphaMask = pattern1 * pattern2;
    
    // Apply a soft falloff near edges (circular gradient)
    alphaMask *= smoothstep(0.0, 0.4, 1.0 - length(uv - 0.5) * 1.4);
    
    // Create the final color
    float4 finalColor = input.color;
    
    // Apply some procedural color variation
    float colorVariation = (sin(distortedUV.x * 5.0f + distortedUV.y * 7.0f) + 1.0) * 0.15f + 0.7f;
    finalColor.rgb = lerp(
        finalColor.rgb,
        finalColor.rgb * colorVariation,
        0.5
    );
    
    // Set alpha based on the procedural pattern and lifetime
    finalColor.a *= alphaMask;
    
    return finalColor;
}
