struct Particle
{
    float3 position;
    float3 velocity;
    float lifetime;
    float maxLifetime;
};

struct VSOutput
{
    float3 position : POSITION;
    float3 velocity : TEXCOORD0;
    float2 lifetime : TEXCOORD1;
    float size : TEXCOORD2;
};

// The particle data from the compute shader
StructuredBuffer<Particle> Particles : register(t0);

// Camera view and projection matrices
cbuffer CameraBuffer : register(b0)
{
    matrix view;
    matrix projection;
    matrix viewProjection;
}

VSOutput main(uint vertexID : SV_VertexID)
{
    VSOutput output;
    
    // Get the particle data
    Particle particle = Particles[vertexID];
    
    // Pass through the position in world space - geometry shader will handle projection
    output.position = particle.position;
    
    // Pass velocity for potential effects
    output.velocity = particle.velocity;
    
    // Pack lifetime and maxLifetime into a float2
    output.lifetime = float2(particle.lifetime, particle.maxLifetime);
    
    // Calculate size based on lifetime (example: particles get smaller as they age)
    float normalizedLife = particle.lifetime / particle.maxLifetime;
    output.size = 0.1f + 0.2f * normalizedLife; // Example size formula
    
    return output;
}