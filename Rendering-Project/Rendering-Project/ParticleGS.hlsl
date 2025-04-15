// ParticleGS.hlsl

// Input from vertex shader
struct GSInput
{
    float3 position : POSITION;
    float3 velocity : TEXCOORD0;
    float2 lifetime : TEXCOORD1;
    float size : TEXCOORD2;
};

// Output to pixel shader
struct GSOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR0;
};

// Camera buffer (same as vertex shader)
cbuffer CameraBuffer : register(b0)
{
    matrix view;
    matrix projection;
    matrix viewProjection;
    float3 cameraPosition;
};

// Creates a billboard quad for each particle
[maxvertexcount(6)]
void main(point GSInput input[1] : POSITION, inout TriangleStream<GSOutput> output)
{
    // Calculate billboard vectors
    float3 particlePos = input[0].position;
    
    // Front vector (from camera to particle)
    float3 frontVec = normalize(particlePos - cameraPosition);
    
    // Default up vector (world up)
    float3 worldUp = float3(0, 1, 0);
    
    // Right vector (perpendicular to front and world up)
    float3 rightVec = normalize(cross(worldUp, frontVec));
    
    // Actual up vector (perpendicular to front and right)
    float3 upVec = cross(frontVec, rightVec);
    
    // Scale by size
    float halfSize = input[0].size * 0.5f;
    rightVec *= halfSize;
    upVec *= halfSize;
    
    // Calculate normalized lifetime for color
    float normalizedLife = input[0].lifetime.x / input[0].lifetime.y;
    
    // Create a color based on lifetime
    float4 particleColor = float4(normalizedLife, normalizedLife, 1.0, normalizedLife);
    
    // Create the vertices of the quad
    GSOutput v[4];
    
    // Top-left
    v[0].position = mul(float4(particlePos - rightVec + upVec, 1.0f), viewProjection);
    v[0].texCoord = float2(0, 0);
    v[0].color = particleColor;
    
    // Bottom-left
    v[1].position = mul(float4(particlePos - rightVec - upVec, 1.0f), viewProjection);
    v[1].texCoord = float2(0, 1);
    v[1].color = particleColor;
    
    // Top-right
    v[2].position = mul(float4(particlePos + rightVec + upVec, 1.0f), viewProjection);
    v[2].texCoord = float2(1, 0);
    v[2].color = particleColor;
    
    // Bottom-right
    v[3].position = mul(float4(particlePos + rightVec - upVec, 1.0f), viewProjection);
    v[3].texCoord = float2(1, 1);
    v[3].color = particleColor;
    
    // First triangle (top-left, bottom-left, top-right)
    output.Append(v[0]);
    output.Append(v[1]);
    output.Append(v[2]);
    output.RestartStrip();
    
    // Second triangle (top-right, bottom-left, bottom-right)
    output.Append(v[2]);
    output.Append(v[1]);
    output.Append(v[3]);
    output.RestartStrip();
}
