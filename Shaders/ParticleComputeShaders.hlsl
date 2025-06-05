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

struct SortData
{
    float rangeSq;
    uint index;
};

struct DeadData
{
    uint index;
};

cbuffer Props : register(b0)
{
    int GroupCount;
    uint MaxParticles;
    float DeltaTime;
	float InitialLifeTime;
};

cbuffer Params : register(b1)
{
    float4x4 View;
    float4x4 Projection;
};

#define THREAD_GROUP_X 32
#define THREAD_GROUP_Y 24
#define THREAD_GROUP_Z 1
#define THREAD_GROUP_TOTAL THREAD_GROUP_X * THREAD_GROUP_Y * THREAD_GROUP_Z

RWStructuredBuffer<Particle> particlePool : register(u0);
AppendStructuredBuffer<SortData> sortListIn : register(u1);
ConsumeStructuredBuffer<SortData> sortListOut : register(u2);
AppendStructuredBuffer<DeadData> deadListIn: register(u3);
ConsumeStructuredBuffer<DeadData> deadListOut: register(u4);

StructuredBuffer<SortData> sortListRead : register(t0);
Texture2DArray shadowMap : register(t1);

[numthreads(THREAD_GROUP_X, THREAD_GROUP_Y, THREAD_GROUP_Z)]
void CSInit(uint3 groupID : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
    uint index = groupID.x * THREAD_GROUP_TOTAL + groupID.y * GroupCount * THREAD_GROUP_TOTAL + groupIndex;

	[flatten]
	if(index >= MaxParticles)
		return;

    DeadData data;
    data.index = index;

    deadListIn.Append(data);
}

[numthreads(THREAD_GROUP_X, THREAD_GROUP_Y, THREAD_GROUP_Z)]
void CSEmit(uint3 groupID : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
    uint index = groupID.x * THREAD_GROUP_TOTAL + groupID.y * GroupCount * THREAD_GROUP_TOTAL + groupIndex;

	[flatten]
	if(index >= MaxParticles)
		return;

    SortData data;
    data.index = deadListOut.Consume().index;
    data.rangeSq = 0.0f;

    sortListIn.Append(data);
}

[numthreads(THREAD_GROUP_X, THREAD_GROUP_Y, THREAD_GROUP_Z)]
void CSUpdate(uint3 groupID : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
    uint index = groupID.x * THREAD_GROUP_TOTAL + groupID.y * GroupCount * THREAD_GROUP_TOTAL + groupIndex;

	[flatten]
	if(index >= MaxParticles)
		return;

    Particle pin = particlePool[sortListRead[index].index];
    pin.pos += float4(pin.velocity, 0.0f);
    particlePool[sortListRead[index].index].posDelta = pin.pos - particlePool[sortListRead[index].index].pos;
    particlePool[sortListRead[index].index].pos = pin.pos;
    particlePool[sortListRead[index].index].velocity += pin.acceleration;
    float4 coords = mul(particlePool[sortListRead[index].index].pos, View);
    //if (shadowMap[0][uint (coords.x / coords.w * 1080)][uint (coords.y / coords.w * 1080)] < coords.z / 80000.0f)
    //    particlePool[sortListRead[index].index].velocity *= -1.0f;
    if (particlePool[sortListRead[index].index].pos.y <= 0.0f)
    {
        particlePool[sortListRead[index].index].velocity *=  (particlePool[sortListRead[index].index].posDelta / abs( particlePool[sortListRead[index].index].posDelta)) / 1.2f;
    }
}

[numthreads(THREAD_GROUP_X, THREAD_GROUP_Y, THREAD_GROUP_Z)]
void CSConsume(uint3 groupID : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
    sortListOut.Consume();
}