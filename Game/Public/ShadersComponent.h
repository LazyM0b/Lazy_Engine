#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <iostream>
#include <wrl.h>

#include "DisplayWin32.h"

class ShadersComponent {
public:

	ShadersComponent();
	int Initialize(HWND hWindow, Microsoft::WRL::ComPtr<ID3D11Device> device, ID3D11DeviceContext* context);
	void DrawOpaque(ID3D11DeviceContext* context);
	void DrawLighting(ID3D11DeviceContext* context);
	void DrawTransparent(ID3D11DeviceContext* context);

	ID3D11InputLayout* opaqueLayout;
	ID3D11InputLayout* lightingLayout;
	ID3D11InputLayout* transparentLayout;
	ID3D11VertexShader* opaqueVertexShader;
	ID3D11VertexShader* lightingVertexShader;
	ID3D11VertexShader* transparentVertexShader;
	ID3DBlob* opaqueVertexShaderByteCode;
	ID3DBlob* lightingVertexShaderByteCode;
	ID3DBlob* transparentVertexShaderByteCode;
	ID3D11PixelShader* opaquePixelShader;
	ID3D11PixelShader* lightingPixelShader;
	ID3D11PixelShader* transparentPixelShader;
	ID3DBlob* opaquePixelShaderByteCode;
	ID3DBlob* lightingPixelShaderByteCode;
	ID3DBlob* transparentPixelShaderByteCode;
	ID3D11RasterizerState* rastState;
	ID3D11SamplerState* textureSampler;
	ID3D11SamplerState* shadowSampler;
};