#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <vector>

#include "LightningComponent.h"
#include "ShadowsComponent.h"
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

class GameComponent;

struct GBuffer
{
	Vector4 posW;
	Vector4 diffuse;
	Vector4 specular;
	Vector3 normal;
};

static const UINT GBufferSize = 6;
class DeferredSystem
{
public:
	void Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device, UINT clientWidth, UINT clientHeight, UINT mapSize);
	void DrawOpaque(ID3D11DeviceContext* context, std::vector<GameComponent*> objects);
	void DrawLighting(ID3D11DeviceContext* context);
	void DrawTransparent(ID3D11DeviceContext* context, std::vector<GameComponent*> objects, UINT mapSize);
	void PrepareFrame();

	ID3D11Texture2D* const* GetTexture(UINT texID) { return &opaqueBuffer[texID]; };
	ID3D11RenderTargetView* const* GetRenderTargets() { return &opaqueRenderView[0]; };
	ID3D11ShaderResourceView* const* GetSRV(UINT texID) { return &opaqueSRV[texID]; };

	ID3D11BlendState* blendState;

	ID3D11Buffer* pointsBuffer;
	ID3D11Texture2D* opaqueBuffer[GBufferSize];
	ID3D11RenderTargetView* opaqueRenderView[GBufferSize];
	ID3D11ShaderResourceView* opaqueSRV[GBufferSize];
	std::vector<UINT>* points;
	std::vector<std::vector<GameComponent*>> transpObjects;
};
