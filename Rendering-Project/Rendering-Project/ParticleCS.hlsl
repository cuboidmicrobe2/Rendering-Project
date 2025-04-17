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

// Random number generation for more natural movement 
float rand(float2 co)
{
    return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);
}

[numthreads(32, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    
    Particle gettingProcessed = Particles[index];
    
    // Apply some random sideways motion to create swirling effect
    float randX = (rand(float2(gettingProcessed.position.x, gettingProcessed.position.y + deltaTime)) - 0.5) * 0.02;
    float randZ = (rand(float2(gettingProcessed.position.z, gettingProcessed.position.y + deltaTime)) - 0.5) * 0.02;
    
    // Add a slow upward drift (smoke rises)
    gettingProcessed.velocity.x += randX;
    gettingProcessed.velocity.z += randZ;
    
    // Dampen horizontal velocity for stability
    gettingProcessed.velocity.x *= 0.99;
    gettingProcessed.velocity.z *= 0.99;
    
    // Apply gentle upward force (smoke rising)
    gettingProcessed.velocity.y += deltaTime * 0.05f;
    
    // Limit maximum velocity for slow ascent
    float maxVelocity = 0.08;
    float currentVelLength = length(gettingProcessed.velocity);
    if (currentVelLength > maxVelocity)
    {
        gettingProcessed.velocity = (gettingProcessed.velocity / currentVelLength) * maxVelocity;
    }
    
    // Update position based on velocity
    gettingProcessed.position += gettingProcessed.velocity * deltaTime * 2.0;
    
    // Slowly decrease lifetime
    gettingProcessed.lifetime -= deltaTime;
    
    // Reset particle when lifetime is over
    if (gettingProcessed.lifetime <= 0.0f)
    {
        // Reset to a random position near the ground in a circle
        float angle = rand(float2(index, deltaTime)) * 6.28; // 0 to 2π
        float radius = rand(float2(deltaTime, index)) * 2.0; // Radius of emission area
        
        gettingProcessed.position = float3(
            radius * cos(angle), // X position in a circle
            0.0f, // Start at ground level
            radius * sin(angle) // Z position in a circle
        );
        
        // Initial small upward velocity with tiny random horizontal component
        gettingProcessed.velocity = float3(
            (rand(float2(index, deltaTime + 1)) - 0.5) * 0.01, // Tiny X velocity
            0.01f + rand(float2(index, deltaTime)) * 0.02, // Small upward Y velocity
            (rand(float2(index, deltaTime + 2)) - 0.5) * 0.01 // Tiny Z velocity
        );
        
        // Randomize lifetime between 5 and 10 seconds for varied effect
        gettingProcessed.lifetime = gettingProcessed.maxLifetime * (0.5 + rand(float2(index, deltaTime)) * 0.5);
    }
    
    Particles[index] = gettingProcessed;
}
