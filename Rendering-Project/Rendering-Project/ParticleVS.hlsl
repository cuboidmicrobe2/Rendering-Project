struct Particle
{
    float3 position : POSITION;
    float3 velocity : VELOCITY;
    float lifetime : LIFETIME;
    float maxLifetime : MAX_LIFETIME;
};

struct VSOutput
{
    float3 position : POSITION;
    float2 lifetime : TEXCOORD0;
    float size : TEXCOORD1;
};

// The particle data from the compute shader
StructuredBuffer<Particle> Particles : register(t0);

VSOutput main(uint vertexID : SV_VertexID)
{
    
    VSOutput output;
    
    // Get the particle data
    Particle particle = Particles[vertexID];
    
    // Pass through the position in world space - geometry shader will handle projection
    output.position = particle.position;
    
    // Pack lifetime and maxLifetime into a float2
    output.lifetime = float2(particle.lifetime, particle.maxLifetime);
    
    // Calculate size based on lifetime (example: particles get smaller as they age)
    float normalizedLife = particle.lifetime / particle.maxLifetime;
    output.size = 0.05f + 0.2f * (1.0f - normalizedLife); // Larger as lifetime decreases
    
    return output;
}