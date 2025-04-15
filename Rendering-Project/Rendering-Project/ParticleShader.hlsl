struct Particle
{
    float3 position;
    float3 velocity;
    float lifetime;
    float maxLifetime;
};

RWStructuredBuffer<Particle> Particles : register(u0);

[numthreads(32, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    
    Particle gettingProcessed = Particles[index];
    
    gettingProcessed.position += gettingProcessed.velocity;
    gettingProcessed.lifetime -= 0.1;
    
    if (gettingProcessed.lifetime <= 0.0)
    {
        gettingProcessed.position = float3(0.0, 0.0, 0.0);
        gettingProcessed.velocity = float3(0.0, 0.0, 0.0);
        gettingProcessed.lifetime = gettingProcessed.maxLifetime;
    }
    
    Particles[index] = gettingProcessed;
}