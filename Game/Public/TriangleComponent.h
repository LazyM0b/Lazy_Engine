#pragma once

#include <d3d11.h>
#include <vector>
#include <wrl.h>

#include "ShadowsComponent.h"
#include "SimpleMath.h"
#include "LightningComponent.h"
#include "DeferredRenderingComponent.h"

using namespace DirectX::SimpleMath;

struct Vertex {
	Vector4 location;
	Vector4 color;
	Vector3 normal;
	Vector2 texCoord;
};

struct objectProps {
	Matrix transformW;
	Matrix transformWInvT;
	Matrix transformH;
	Material material;
};

//SHADOWS
struct shadowVertex {
	Vector4 location;
};

enum MeshTypes
{
	Strip,
	Triangle,
	Square,
	Circle,
	Cube,
	Sphere,
	Mesh
};

class TriangleComponent {
public:

	TriangleComponent();
	void Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device);
	void DrawTriangle(ID3D11DeviceContext* context);
	//SHADOWS
	void DrawShadowTriangle(ID3D11DeviceContext* context);
	//END
	void DrawLine(ID3D11DeviceContext* context);
	void ComputeNormal(const Vector3& p1, const Vector3& p2, const Vector3& p3, Vector3& outN);

	UINT strides;
	UINT offsets;
	MeshTypes type;
	std::vector <Vertex>* points;
	std::vector <shadowVertex>* shadowPoints;
	std::vector <int>* indexes;
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	ID3D11Buffer* objectPropsBuffer;
	//SHADOWS
	UINT shadowStrides;
	UINT shadowOffsets;
	ID3D11Buffer* shadowVertexBuffer;
	//END
};