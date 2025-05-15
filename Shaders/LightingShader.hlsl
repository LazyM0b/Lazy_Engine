struct VS_IN
{
	uint pos : POSITION0;
};

struct PS_IN
{
	float4 pos :	SV_POSITION;
	float2 tex :	TEXCOORD;
};

struct Material
{
	float4 ambient;
    float4 diffuse;
    float4 specular;
};

struct DirectionalLight 
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float4 direction;
};

struct PointLight 
{
	float4 diffuse;
	float4 specular;

	float4 position;
	float4 attenuation;
};

static const int CascadesCount = 4;

cbuffer Lightning : register(b0)
{
	DirectionalLight dirLight;
	PointLight pointLight[800];
	float4 eyePos;
	float4x4 transformHInv;
    float4 data;
};

cbuffer Cascades : register(b1)
{
	float4x4 transformVP[CascadesCount];
	float4x4 transformS[CascadesCount];
	float4 distances[CascadesCount / 4];
	float4 playerPos;
};
Texture2D<float4> PosW : register(t0);
Texture2D<float4> PosH : register(t1);
Texture2D<float4> Ambient : register(t2);
Texture2D<float4> Diffuse : register(t3);
Texture2D<float4> Specular : register(t4);
Texture2D<float4> Normal : register(t5);

Texture2DArray shadowMap : TEXTUREARRAY : register(t6);

SamplerState objSampler : SAMPLER : register(s0);

SamplerComparisonState shadowSampler : SAMPLER : register(s1);


void ComputeDirectionalLight (float4 difColor, float4 specColor, DirectionalLight light, float3 norm, float3 eye, out float4 ambient, out float4 diffuse, out float4 specular) 
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightVector = -light.direction.xyz;

	ambient = light.ambient;

	float dif = dot(lightVector, norm);
	
	[FLATTEN] if (dif > 0.0f)
	{
		float3 v = reflect(-lightVector, norm);
		float spec = pow(max(dot(v, eye), 0.0f), specColor.w);

		diffuse = dif * difColor * light.diffuse;
		specular = spec * specColor * light.specular;
	}
}

void ComputePointLight (float4 difColor, float4 specColor, PointLight light, float3 pos, float3 norm, float3 eye, out float4 diffuse, out float4 specular) 
{
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightVector = light.position.xyz - pos;
	float d = length(lightVector);


	if (d > light.attenuation.w)
		return;

	lightVector /= d;
	
	float dif = dot(lightVector, norm);
	
	[FLATTEN] if (dif > 0.0f)
	{
		float3 v = reflect(-lightVector, norm);
		float spec = pow(max(dot(v, eye), 0.0f), specColor.w);

		diffuse = dif * difColor * light.diffuse;
		specular = spec * specColor * light.specular;
	}
	float attenuation = 1.0f / dot(light.attenuation.xyz, float3(1.0f, d, d*d));
	
	diffuse *= attenuation;
	specular *= attenuation;

	//for gradient
	if (d > light.attenuation.w / 2)
		specular *= 1.0f - (d - light.attenuation.w / 2) / (light.attenuation.w / 2);
}

//SHADOWS
static const float SHADOWMAP_SIZE = 8192.0f;

static const float SHADOWMAP_DX = 1.0f / SHADOWMAP_SIZE;

float CalcShadowFactor(SamplerComparisonState shadowSampler, Texture2DArray shadowMap, float4 pos, int layer)
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
        percent += shadowMap.SampleCmp(shadowSampler, float3(pos.xy + offsets[i], layer), depth).r;
    };

	return percent /= 9.0f;
}
//END

PS_IN VSMain( VS_IN input)
{
	PS_IN output = (PS_IN)0;

	output.tex = float2((int)input.pos % 800, (int)input.pos / 800);
	output.pos = float4(output.tex / 800 * float2(2, -2) + float2(-1, 1), 0, 1);
	return output;
}

float4 PSMain( PS_IN input ) : SV_Target
{
	float4 posN = PosH.Load(int3(input.tex, 0));
	posN = float4(posN.xyz / posN.w, posN.w);
	float4 posW =  PosW.Load(int3(input.tex, 0));
	//float4 posW =  mul(posN, transformHInv);
	float4 ambColor = Ambient.Load(int3(input.tex, 0));
	float4 difColor = Diffuse.Load(int3(input.tex, 0));
	float4 specColor = Specular.Load(int3(input.tex, 0));
	float4 norm = Normal.Load(int3(input.tex, 0));
	
	/*
	if (input.tex.x != 0.0f && input.tex.y != 0.0f) 
		col = objTexture.Sample(objSampler, input.tex);
	*/

	float4 col;
	if ((dirLight.diffuse.w != 0.0f || pointLight[0].diffuse.w != 0.0f) && (difColor.x + difColor.y + difColor.z != 3.0f)) {
		float3 eyeVec = normalize(float3(eyePos.xyz) - posW.xyz);
        
		float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
		float4 amb, dif, spec;

		int layer = CascadesCount;

		float depthVal;
		float distance;

		for (int i = 0; i < CascadesCount; ++i)
		{
			depthVal = sqrt(pow(posW.x - playerPos.x, 2) + pow(posW.z - playerPos.z, 2));

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

		float4 posS = mul(posW, transformS[layer]);
		float4 shadow = float4 (1.0f, 1.0f, 1.0f, 1.0f);

		shadow *= CalcShadowFactor(shadowSampler, shadowMap, posS, layer);

		ComputeDirectionalLight(difColor, specColor, dirLight, norm.xyz, eyeVec.xyz, amb, dif, spec);

		//Only for cascade shadows debugging
		//#define COLORING
		
		#ifdef COLORING
		float4 cascadeColor;
		if (layer == 0)
		{
			cascadeColor = float4(1.0f, 0.5f, 0.5f, 1.0f);
		}
		else if (layer == 1)
		{
			cascadeColor = float4(1.0f, 1.0f, 0.5f, 1.0f);
		}
		else if (layer == 2)
		{
			cascadeColor = float4(0.5f, 1.0f, 0.5f, 1.0f);
		}
		else if (layer == 3)
		{
			cascadeColor = float4(0.5f, 0.5f, 1.0f, 1.0f);
		}

		shadow *= cascadeColor;
		amb *= cascadeColor;
		#endif
		
		if (layer == CascadesCount)
			shadow = 1.0f;

		ambient = ambColor * amb;
		diffuse += dif * shadow;
		specular += spec * shadow;

        for (int i = 0; i < (int)data.x; ++i)
        {
            ComputePointLight(difColor, specColor, pointLight[i], posW.xyz, norm.xyz, eyeVec.xyz, dif, spec);

            diffuse += dif;
            specular += spec;
        }
		

       col =  ambient + diffuse + specular;
		
		col.w = difColor.w;
    }

	if ((int)data.y == 2)
		col = posN;
	else if ((int)data.y == 3)
		col = difColor;
	else if ((int)data.y == 4)
		col = specColor;
	else if ((int)data.y == 5)
		col = norm;
	return col;
}