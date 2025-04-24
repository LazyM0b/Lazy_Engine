#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <iostream>
#include <wrl.h>
#include "SimpleMath.h"

#include "DisplayWin32.h"
using namespace DirectX::SimpleMath;
//SHADOWS
class ShadowMap 
{
public:
	ShadowMap(Microsoft::WRL::ComPtr<ID3D11Device> device, UINT Width, UINT Height);
	int ShadersInitialize(HWND hWindow, Microsoft::WRL::ComPtr<ID3D11Device> device, ID3D11DeviceContext* context);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetDepthMapSRV();

	void BindDsvAndSetNullRenderTarget(ID3D11DeviceContext* context);

private:
	ShadowMap(const ShadowMap& rhs);
	ShadowMap& operator=(const ShadowMap& rhs);

	UINT mapWidth;
	UINT mapHeight;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> depthMapSRV;
	ID3D11DepthStencilView* depthMapDSV;

	D3D11_VIEWPORT sViewport;

	ID3D11InputLayout* layout;
	ID3D11VertexShader* vertexShader;
	ID3DBlob* vertexShaderByteCode;
	ID3D11GeometryShader* geometryShader;
	ID3DBlob* geometryShaderByteCode;
	ID3D11RasterizerState* rastState;
};

struct BoundingSphere
{
	BoundingSphere() : center(0.0f, 0.0f, 0.0f), radius(0.0f) {};
	Vector3 center;
	float radius;
};


static const UINT CascadesCount = 4;

struct shadowMapProps {
	shadowMapProps() {};
	shadowMapProps(const shadowMapProps& other)
	{
		for (int i = 0; i < CascadesCount; ++i)
		{
			transformVP[i] = other.transformVP[i];
			distances[i] = other.distances[i];
		}
	};

	Matrix transformV[CascadesCount];
	Matrix transformVP[CascadesCount];
	Matrix transformS[CascadesCount];
	Vector4 distances[CascadesCount / 4];
	Vector4 objectPos;
};
//END