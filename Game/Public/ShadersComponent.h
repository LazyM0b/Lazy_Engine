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
	void Draw(ID3D11DeviceContext* context);

	ID3D11InputLayout* layout;
	ID3D11VertexShader* vertexShader;
	ID3DBlob* vertexShaderByteCode;
	ID3D11PixelShader* pixelShader;
	ID3DBlob* pixelShaderByteCode; 
	ID3D11RasterizerState* rastState;
	ID3D11SamplerState* textureSampler;
	ID3D11SamplerState* shadowSampler;
};