struct Particle
{
    float4 color;
    float4 colorDelta;
    float4 pos;
    float4 posDelta;
    float3 velocity;
    float3 acceleration;
    float2 size;
    float2 sizeDelta;
    float lifeTime;
    float weight;
};

struct DeadData
{
    uint index;
};

struct SortData
{
    float rangeSq;
    uint index;
};

struct PS_IN
{
    float4 pos: SV_POSITION;
    float4 color: COLOR;
};

#define BITONIC_BLOCK_SIZE 512

StructuredBuffer<Particle> particlePool : register(t0);
StructuredBuffer<SortData> sortList : register(t1);
ConsumeStructuredBuffer<DeadData> deadListIn: register(u2);
AppendStructuredBuffer<DeadData> deadListOut: register(u3);

cbuffer Params : register(b0)
{
    float4x4 View;
    float4x4 Projection;
};

PS_IN VSMain(uint VertexID: SV_VertexID)
{
    PS_IN vout;

    uint index = sortList[VertexID / 4].index;

    Particle pin = particlePool[index];

    vout.pos = pin.pos;

    uint corner = VertexID % 4;

    //float pSize = lerp(pin.size.x, pin.size.y, saturate(1.0f - pin.LifeTime / pin.MaxLifeTime));

    [flatten]
    if (corner == 0)
    {
        vout.pos += float4(pin.size.x, pin.size.y, 0.0f, 0.0f);
    }

    [flatten]
    if (corner == 1)
    {
        vout.pos += float4(-pin.size.x, -pin.size.y, 0.0f, 0.0f);
    }

    [flatten]
    if (corner == 2)
    {
        vout.pos += float4(pin.size.x, -pin.size.y, 0.0f, 0.0f);
    }

    [flatten]
    if (corner == 3)
    {
        vout.pos += float4(-pin.size.x, pin.size.y, 0.0f, 0.0f);
    }
    
    vout.pos = mul(vout.pos, View);
    vout.pos = mul(vout.pos, Projection);
    vout.pos.z /= 80000.0f;
    vout.color = pin.color;

    return vout;
}

float4 PSMain( PS_IN input) : SV_TARGET
{
    float4 pout = input.color;
   // pout /= 10.0f;

    return pout;
}

[numthreads(BITONIC_BLOCK_SIZE, 1, 1)]
void CSMain(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
    
}
