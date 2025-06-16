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
	float4 posH : SV_POSITION;
	float4 posW : POSITION;
 	float4 col : COLOR;
	float2 tex : TEXCOORD0;
	float3 norm: NORMAL;
    Material mat : MATERIAL_DATA;
};

struct GBuffer
{
    float4 posH : SV_Target0;
    float4 posW : SV_Target1;
    float4 ambient : SV_Target2;
    float4 diffuse : SV_Target3;
    float4 specular : SV_Target4;
    float4 normal : SV_Target5;
};

static const int CascadesCount = 4;

cbuffer Properties : register(b0)
{
	float4x4 transformW;
	float4x4 transformWInvT;
	float4x4 transformH;
	Material mat;
};

Texture2D objTexture : TEXTURE : register(t0);

SamplerState objSampler : SAMPLER : register(s0);

PS_IN VSMain( VS_IN input)
{
	PS_IN output = (PS_IN)0;
    output.posW = mul(input.pos, transformW);
    output.posH = mul(output.posW, transformH);
    //output.posH.z /= 20000.0f;
    output.col = input.col;
    output.tex = input.tex;
    output.norm = mul(input.norm, (float3x3) transformWInvT);
    output.mat = mat;
    return output;
}

GBuffer PSMain( PS_IN input ) : SV_Target
{
    GBuffer outData;
    	
	if (input.tex.x != 0.0f && input.tex.y != 0.0f) 
        input.col = objTexture.Sample(objSampler, input.tex);
    input.col = float4(pow(input.col.x, 1/2.2f), pow(input.col.y, 1/2.2f), pow(input.col.z, 1/2.2f), 1.0f);

    input.norm = normalize(input.norm);
	        
    outData.posW = input.posW;
    outData.posH = input.posH;
    outData.ambient = input.col;
    outData.diffuse = input.mat.diffuse;
    //outData.diffuse = float4(pow(outData.diffuse.x, 1/2.2f), pow(outData.diffuse.y, 1/2.2f), pow(outData.diffuse.z, 1/2.2f), 1.0f);
    outData.specular = input.mat.specular;
    outData.normal = float4(input.norm, 1.0f);
    
    return outData;
}