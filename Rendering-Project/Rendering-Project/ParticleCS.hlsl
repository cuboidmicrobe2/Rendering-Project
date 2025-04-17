struct Particle
{
    float3 position;
    float3 velocity;
    float lifetime;
    float maxLifetime;
};

cbuffer TimeBuffer : register(b2)
{
    float deltaTime;
    float3 padding;
}

RWStructuredBuffer<Particle> Particles : register(u1);

[numthreads(32, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    
    Particle gettingProcessed = Particles[index];
    
    gettingProcessed.position += gettingProcessed.velocity;
    gettingProcessed.velocity.y += deltaTime * 0.5f;
    gettingProcessed.lifetime -= deltaTime;
    
    if (gettingProcessed.lifetime <= 0.0f)
    {
        gettingProcessed.position = float3(0.0f, 0.0f, 0.0f);
        gettingProcessed.velocity = float3(0.0f, 0.0f, 0.0f);
        gettingProcessed.lifetime = gettingProcessed.maxLifetime;
    }
    
    Particles[index] = gettingProcessed;
}