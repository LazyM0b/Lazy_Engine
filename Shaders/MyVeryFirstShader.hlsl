struct VS_IN
{
	float4 pos : POSITION0;
	float4 col : COLOR0;
	float3 norm: NORMAL;
	float2 tex : TEXCOORD;
};

struct Material 
{
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
	Material mat : MATERIAL;
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

struct SpotLight 
{
	float4 diffuse;
	float4 specular;

	float4 position;
	float4 attenuation;
	float4 cone;
};

static const int CascadesCount = 4;

cbuffer Properties : register(b0)
{
	float4x4 transformW;
	float4x4 transformWInvT;
	float4x4 transformH;
	Material mat;
};

cbuffer Lightning : register(b1)
{
	DirectionalLight dirLight;
	PointLight pointLights[400];
	SpotLight spotLights[400];
	float4 eyePos;
	float4x4 transformHInv;
    float4 data;
};

cbuffer Cascades : register(b2)
{
	float4x4 transformVP[CascadesCount];
	float4x4 transformS[CascadesCount];
	float4 distances[CascadesCount / 4];
	float4 playerPos;
};

Texture2D objTexture : TEXTURE : register(t0);

Texture2DArray shadowMap : TEXTUREARRAY : register(t1);

SamplerState objSampler : SAMPLER : register(s0);

SamplerComparisonState samplerShadow : SAMPLER : register(s1);


void ComputeDirectionalLight (Material mat, DirectionalLight light, float3 norm, float3 eye, out float4 ambient, out float4 diffuse, out float4 specular) 
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
		float spec = pow(max(dot(v, eye), 0.0f), mat.specular.w);

		diffuse = dif * mat.diffuse * light.diffuse;
		specular = spec * mat.specular * light.specular;
	}
}

void ComputePointLight (Material mat, PointLight light, float3 pos, float3 norm, float3 eye, out float4 diffuse, out float4 specular) 
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
		float spec = pow(max(dot(v, eye), 0.0f), mat.specular.w);

		diffuse = dif * mat.diffuse * light.diffuse;
		specular = spec * mat.specular * light.specular;
	}
	float attenuation = 1.0f / dot(light.attenuation.xyz, float3(1.0f, d, d*d));
	
	diffuse *= attenuation;
	specular *= attenuation;

	//for gradient
	if (d > light.attenuation.w / 2)
		specular *= 1.0f - (d - light.attenuation.w / 2) / (light.attenuation.w / 2);
}

void ComputeSpotLight (Material mat, SpotLight light, float3 pos, float3 norm, float3 eye, out float4 diffuse, out float4 specular) 
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
		float spec = pow(max(dot(v, eye), 0.0f), mat.specular.w);

		diffuse = dif * mat.diffuse * light.diffuse;
		specular = spec * mat.specular * light.specular;
	}
	float spot = pow(max(dot(-lightVector, light.cone.xyz), 0.0f), light.cone.w);
	float attenuation = spot / dot(light.attenuation.xyz, float3(1.0f, d, d*d));
	
	diffuse *= attenuation;
	specular *= attenuation;

	//for gradient
	//if (d > light.attenuation.w / 2)
	//	specular *= 1.0f - (d - light.attenuation.w / 2) / (light.attenuation.w / 2);
}

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
	percent /= 9.0f;

	return percent;
}
//END

PS_IN VSMain( VS_IN input)
{
	PS_IN output = (PS_IN)0;

	output.posW = mul(input.pos, transformW);
	output.posH = mul(output.posW, transformH);
    output.posH.z = output.posH.z / 80000.0f;
	output.col = input.col;
	output.tex = input.tex;
	output.norm = mul(input.norm, (float3x3)transformWInvT);
	output.mat = mat;
	return output;
}

float4 PSMain( PS_IN input ) : SV_Target
{
    //input.posH = float4(input.posH.xyz / input.posH.w, 1);
    
	float4 col = input.col;
	if (input.tex.x != 0.0f && input.tex.y != 0.0f) 
		col = objTexture.Sample(objSampler, input.tex);

	if ((dirLight.ambient.w != 0.0f || pointLights[0].diffuse.w != 0.0f || spotLights[0].diffuse.w != 0.0f) && (col.x + col.y + col.z != 3.0f)) {
		input.norm = normalize(input.norm);

		float3 eyeVec = normalize(eyePos.xyz - input.posW.xyz);
        
		float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
		float4 amb, dif, spec;

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
		float4 shadow = float4 (1.0f, 1.0f, 1.0f, 1.0f);

		shadow *= CalcShadowFactor(samplerShadow, shadowMap, posS, layer);

		ComputeDirectionalLight(input.mat, dirLight, input.norm, eyeVec, amb, dif, spec);

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

		ambient += amb;
		diffuse += dif * shadow;
		specular += spec * shadow;
		
        for (int i = 0; i < data.x; ++i)
        {
            ComputePointLight(input.mat, pointLights[i], input.posW.xyz, input.norm.xyz, eyeVec.xyz, dif, spec);

            diffuse += dif;
            specular += spec;
        }
		
        for (int i = 0; i < (int)data.y; ++i)
        {
            ComputeSpotLight(input.mat, spotLights[i], input.posW.xyz, input.norm.xyz, eyeVec.xyz, dif, spec);

            diffuse += dif;
            specular += spec;
        }

        float4 matColor; 
		matColor =  col * ambient + diffuse + specular;
		
        col.xyz = matColor.xyz;
		col.w = input.mat.diffuse.w;
    }

	return col;
}