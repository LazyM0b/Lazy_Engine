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
	void DrawParticles(ID3D11DeviceContext* context);
	void SortParticles(ID3D11DeviceContext* context);
	void TransposeParticles(ID3D11DeviceContext* context);

	//Opaque shader
	ID3D11InputLayout* opaqueLayout;
	ID3D11VertexShader* opaqueVertexShader;
	ID3DBlob* opaqueVertexShaderByteCode;
	ID3D11PixelShader* opaquePixelShader;
	ID3DBlob* opaquePixelShaderByteCode;

	//Lighting shader
	ID3D11InputLayout* transparentLayout;
	ID3D11VertexShader* transparentVertexShader;
	ID3DBlob* transparentVertexShaderByteCode;
	ID3D11PixelShader* transparentPixelShader;
	ID3DBlob* transparentPixelShaderByteCode;

	//Transparent shader
	ID3D11InputLayout* lightingLayout;
	ID3D11VertexShader* lightingVertexShader;
	ID3DBlob* lightingVertexShaderByteCode;
	ID3D11PixelShader* lightingPixelShader;
	ID3DBlob* lightingPixelShaderByteCode;

	//Particles shader
	ID3D11InputLayout* particlesLayout;
	ID3D11VertexShader* particlesVertexShader;
	ID3DBlob* particlesVertexShaderByteCode;
	ID3D11PixelShader* particlesPixelShader;
	ID3DBlob* particlesPixelShaderByteCode;
	ID3D11ComputeShader* particlesComputeShader;
	ID3DBlob* particlesComputeShaderByteCode;
	ID3D11ComputeShader* particlesComputeSortShader;
	ID3DBlob* particlesComputeSortShaderByteCode;
	ID3D11ComputeShader* particlesComputeTransposeShader;
	ID3DBlob* particlesComputeTransposeShaderByteCode;

	ID3D11RasterizerState* rastState;
	ID3D11SamplerState* textureSampler;
	ID3D11SamplerState* shadowSampler;
};