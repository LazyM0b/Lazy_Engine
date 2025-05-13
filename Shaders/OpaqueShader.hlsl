struct VS_IN
{
	float4 pos : POSITION0;
	float4 col : COLOR0;
	float3 norm: NORMAL;
	float2 tex : TEXCOORD;
};

struct Material
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
};

struct PS_IN
{
	float4 posW : POSITION;
	float4 posH : SV_POSITION;
 	float4 col : COLOR;
	float3 norm: NORMAL;
	float2 tex : TEXCOORD0;
    Material mat : MATERIAL_DATA;
};

struct GBuffer
{
    float4 ambient : SV_Target0;
    float4 diffuse : SV_Target1;
    float4 specular : SV_Target2;
    float4 posW : SV_Target3;
    float4 normal : SV_Target4;
};

static const int CascadesCount = 4;

cbuffer Properties : register(b0)
{
	float4x4 transformW;
	float4x4 transformWInvT;
	float4x4 transformH;
	Material mat;
};

cbuffer Cascades : register(b2)
{
	float4x4 transformVP[CascadesCount];
	float4x4 transformS[CascadesCount];
	float4 distances[CascadesCount / 4];
	float4 playerPos;
};

Texture2D objTexture : TEXTURE : register(t0);

SamplerState objSampler : SAMPLER : register(s0);

Texture2DArray shadowMap : TEXTUREARRAY : register(t1);

SamplerComparisonState samplerShadow : SAMPLER : register(s1);


//SHADOWS
static const float SHADOWMAP_SIZE = 8192.0f;

static const float SHADOWMAP_DX = 1.0f / SHADOWMAP_SIZE;

float CalcShadowFactor(SamplerComparisonState samplerShadow, Texture2DArray shadowMap, float4 pos, int layer)
{
	pos.xyz /= pos.w;

	float depth = pos.z;

	const float dx = SHADOWMAP_DX;

	const float2 offsets[9] =
	{
		float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx, dx), float2(0.0f, dx), float2(dx, dx)
	};

	float percent = 0.0f;

	[unroll]
	for(int i = 0; i < 9; ++i)
    {
        percent += shadowMap.SampleCmp(samplerShadow, float3(pos.xy + offsets[i], layer), depth).r;
    };

	return percent /= 9.0f;
}
//END

PS_IN VSMain( VS_IN input)
{
	PS_IN output = (PS_IN)0;
    
    output.posW = mul(input.pos, transformW);
    output.posH = mul(output.posW, transformH);
    output.col = input.col;
    output.tex = input.tex;
    output.norm = mul(input.norm, (float3x3) transformWInvT);
    output.mat = mat;
    return output;
}

GBuffer PSMain( PS_IN input ) : SV_Target
{
    GBuffer outData;
    
	float4 col = input.col;
	
	if (input.tex.x != 0.0f && input.tex.y != 0.0f) 
        col = objTexture.Sample(objSampler, input.tex);
    
    input.norm = normalize(input.norm);
	
    int layer = CascadesCount;

    float depthVal;
    float distance;

    for (int i = 0; i < CascadesCount; ++i)
    {
        depthVal = sqrt(pow(input.posW.x - playerPos.x, 2) + pow(input.posW.z - playerPos.z, 2));

        switch (i % 4)
        {
            case 0:
                distance = distances[i / 4].x;
                break;
            case 1:
                distance = distances[i / 4].y;
                break;
            case 2:
                distance = distances[i / 4].z;
                break;
            case 3:
                distance = distances[i / 4].w;
                break;
        }
			
        if (depthVal < distance)
        {
            layer = i;
            break;
        }
    }

    float4 posS = mul(input.posW, transformS[layer]);
    float4 shadow = float4(1.0f, 1.0f, 1.0f, 1.0f);

    shadow *= CalcShadowFactor(samplerShadow, shadowMap, posS, layer);
    shadow.w = 1.0f;
    
    if (layer == CascadesCount)
        shadow = 1.0f;
    
    input.mat.ambient *= col;
    
    outData.ambient = input.mat.ambient;
    outData.diffuse = input.mat.diffuse;
    outData.specular = input.mat.specular;
    outData.posW = input.posW;
    outData.normal = float4(input.norm, 1.0f);
    
    return outData;
}