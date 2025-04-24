struct VertexIn
{
    float4 PosL : POSITION;
};

struct GS_In
{
    float4 PosW : POSITION;
};

struct GS_Out
{
    float4 pos : SV_POSITION;
    uint arrInd : SV_RenderTargetArrayIndex;
};

static const int CascadesCount = 4;

struct Material 
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float4 reflection;
};

cbuffer Properties : register(b0)
{
	float4x4 transformW;
	float4x4 transformWInvT;
	float4x4 transformH;
	Material mat;
};

cbuffer Cascades : register(b1)
{
	float4x4 transformVP[CascadesCount];
	float4x4 transformS[CascadesCount];
	float4 distances[CascadesCount / 4];
    float4 objPos;
};

GS_In VS(VertexIn VIn)
{
    GS_In VOut;
    VOut.PosW = mul(VIn.PosL, transformW);

    return VOut;
}

[instance(CascadesCount)]
[maxvertexcount(3)]
void GS(triangle GS_In p[3], in uint id : SV_GSInstanceID, inout TriangleStream<GS_Out> stream)
{
    GS_Out gs = (GS_Out)0;

    [unroll]
    for (int i = 0; i < 3; ++i)
    {
        gs.pos = mul(p[i].PosW, transformVP[id]);
        gs.arrInd = id;
        stream.Append(gs);
    }
}