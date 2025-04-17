// Input from vertex shader
struct GSInput
{
    float3 position : POSITION;
    float2 lifetime : TEXCOORD0;
    float size : TEXCOORD1;
};

// Output to pixel shader
struct GSOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR0;
};

// Camera buffer (same as vertex shader)
cbuffer CameraBuffer : register(b1)
{
    matrix viewProjection;
    float3 cameraPosition;
};

// Creates a billboard quad for each particle
[maxvertexcount(6)]
void main(point GSInput input[1] : POSITION, inout TriangleStream<GSOutput> output)
{
    // Create color based on lifetime
    float4 particleColor = float4(0, 1, 0, 1);
    
    // Calculate billboard vectors
    float3 particlePos = input[0].position;
    float halfSize = input[0].size * 0.5f;
    
    float3 up = float3(0, 1, 0);
    float3 right = float3(1, 0, 0);
    
    // Get view space directions
    float3 look = normalize(cameraPosition - particlePos);
    
    right = normalize(cross(up, look));
    up = cross(look, right);
    
    // Scale vectors by particle size
    right *= halfSize;
    up *= halfSize;
    
    // Calculate normalized lifetime for color
    float normalizedLife = input[0].lifetime.x / input[0].lifetime.y;
    
    GSOutput vertices[4];
    
    // Top left
    vertices[0].position = mul(float4(particlePos - right + up, 1.0f), viewProjection);
    vertices[0].texCoord = float2(0, 0);
    vertices[0].color = particleColor;

    // Bottom left
    vertices[1].position = mul(float4(particlePos - right - up, 1.0f), viewProjection);
    vertices[1].texCoord = float2(0, 1);
    vertices[1].color = particleColor;

    // Top right
    vertices[2].position = mul(float4(particlePos + right + up, 1.0f), viewProjection);
    vertices[2].texCoord = float2(1, 0);
    vertices[2].color = particleColor;

    // Bottom right
    vertices[3].position = mul(float4(particlePos + right - up, 1.0f), viewProjection);
    vertices[3].texCoord = float2(1, 1);
    vertices[3].color = particleColor;
    
    // First triangle (top left, bottom left, top right)
    output.Append(vertices[0]);
    output.Append(vertices[1]);
    output.Append(vertices[2]);
    output.RestartStrip();
    
    // Second triangle (top right, bottom left, bottom right)
    output.Append(vertices[2]);
    output.Append(vertices[1]);
    output.Append(vertices[3]);
    output.RestartStrip();
}
