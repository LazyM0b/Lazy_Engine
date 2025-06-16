struct Particle
{
    float4 color;
    float4 colorDelta;
    float4 origin;
    float4 pos;
    float4 posDelta;
    float3 initialVelocity;
    float3 velocity;
    float3 acceleration;
    float2 size;
    float2 sizeDelta;
    float lifeTime;
    float weight;
};

struct SortData
{
    uint index;
    float rangeSq;
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
#define THREAD_GROUP_TOTAL 768

RWStructuredBuffer<Particle> particlePool : register(u0);
AppendStructuredBuffer<SortData> sortListIn : register(u1);
ConsumeStructuredBuffer<SortData> sortListOut : register(u2);
AppendStructuredBuffer<DeadData> deadListIn: register(u3);
ConsumeStructuredBuffer<DeadData> deadListOut: register(u4);

StructuredBuffer<SortData> sortListRead : register(t0);
Texture2DArray<float4> shadowMap : register(t1);
Texture2D<float4> Normal : register(t2);

SamplerState objSampler : SAMPLER : register(s0);

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
    
    DeadData deadInd = deadListOut.Consume();

    SortData data;
    data.index = deadInd.index;
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
    /*if (pin.pos.y <= 0.0f)
    {
        pin.pos = float4(0.0f, 100.0f, 0.0f, 0.0f);
    }
    else
    {*/

    pin.pos += float4(pin.velocity, 0.0f);
    particlePool[sortListRead[index].index].posDelta = pin.pos - particlePool[sortListRead[index].index].pos;
    //}
    particlePool[sortListRead[index].index].pos = pin.pos;

    if (particlePool[sortListRead[index].index].lifeTime <= 0.0f)
    {
        particlePool[sortListRead[index].index].lifeTime = InitialLifeTime;
        particlePool[sortListRead[index].index].pos = particlePool[sortListRead[index].index].origin;
        particlePool[sortListRead[index].index].velocity = particlePool[sortListRead[index].index].initialVelocity;
    }

    particlePool[sortListRead[index].index].lifeTime -= DeltaTime;

    particlePool[sortListRead[index].index].velocity.y -= 0.1f;
    float4 coords = mul(particlePool[sortListRead[index].index].pos, View);
    float3 uv = float3((coords.x + coords.w) / coords.w * 0.5f, (coords.y + coords.w) / coords.w * 0.5f, 0.0f);

    /*if (uv.x < 1.0f && uv.y < 1.0f && uv.x > 0.0f && uv.y > 0.0f)
    {
        float depth = shadowMap.SampleLevel(objSampler, uv, 3).r;
        float4 norm = Normal.SampleLevel(objSampler, uv, 0);
        //if (depth < coords.z / 20000.0f - 0.01f)
        if (pin.pos.y < 0.0f)
        {
            particlePool[sortListRead[index].index].velocity = reflect( float4(particlePool[sortListRead[index].index].velocity, 0.0f), norm);
            particlePool[sortListRead[index].index].pos.y += particlePool[sortListRead[index].index].velocity;
        }
    }*/
    if (particlePool[sortListRead[index].index].pos.y <= 0.0f)
    {
        particlePool[sortListRead[index].index].velocity.y *=  (particlePool[sortListRead[index].index].posDelta.y / abs( particlePool[sortListRead[index].index].posDelta.y)) / 1.2f;
    }
}

[numthreads(THREAD_GROUP_X, THREAD_GROUP_Y, THREAD_GROUP_Z)]
void CSConsume(uint3 groupID : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
    uint index = groupID.x * THREAD_GROUP_TOTAL + groupID.y * GroupCount * THREAD_GROUP_TOTAL + groupIndex;

    SortData data1 = sortListOut.Consume();

    if (particlePool[data1.index].lifeTime <= InitialLifeTime)
    {
        DeadData data2;
        data2.index = data1.index;
        deadListIn.Append(data2);
        sortListIn.Append(data1);
    }
}