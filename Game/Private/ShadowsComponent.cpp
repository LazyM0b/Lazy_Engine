#include "ShadowsComponent.h"

//SHADOWS
ShadowMap::ShadowMap(Microsoft::WRL::ComPtr<ID3D11Device> device, UINT Width, UINT Height):mapWidth(Width), mapHeight(Height), depthMapSRV(0), depthMapDSV(0)
{
	sViewport.TopLeftX = 0.0f;
	sViewport.TopLeftY = 0.0f;
	sViewport.Width = static_cast<float>(Width);
	sViewport.Height = static_cast<float>(Height);
	sViewport.MinDepth = 0.0f;
	sViewport.MaxDepth = 1.0f;

	//NEW
	D3D11_TEXTURE2D_DESC TexDesc;
	TexDesc.Width = Width;
	TexDesc.Height = Height;
	TexDesc.MipLevels = 1;
	TexDesc.ArraySize = CascadesCount;
	TexDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	TexDesc.SampleDesc.Count = 1;
	TexDesc.SampleDesc.Quality = 0;
	TexDesc.Usage = D3D11_USAGE_DEFAULT;
	TexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	TexDesc.CPUAccessFlags = 0;
	TexDesc.MiscFlags = 0;

	ID3D11Texture2D* DepthMap = 0;

	HRESULT hr = (device->CreateTexture2D(&TexDesc, 0, &DepthMap));

	D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc;
	DSVDesc.Flags = 0;
	DSVDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	DSVDesc.Texture2DArray.MipSlice = 0;
	DSVDesc.Texture2DArray.FirstArraySlice = 0;
	DSVDesc.Texture2DArray.ArraySize = CascadesCount;

	HRESULT(device->CreateDepthStencilView(DepthMap, &DSVDesc, &depthMapDSV));

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	SRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	SRVDesc.Texture2DArray.MostDetailedMip = 0;
	SRVDesc.Texture2DArray.MipLevels = TexDesc.MipLevels;
	SRVDesc.Texture2DArray.FirstArraySlice = 0;
	SRVDesc.Texture2DArray.ArraySize = CascadesCount;

	HRESULT(device->CreateShaderResourceView(DepthMap, &SRVDesc, &depthMapSRV));
	//
}

int ShadowMap::ShadersInitialize(HWND hWindow, Microsoft::WRL::ComPtr<ID3D11Device> device, ID3D11DeviceContext* context) 
{

	ID3DBlob* errorVertexCode = nullptr;

	auto res = D3DCompileFromFile(L"./Shaders/ShadowMapShader.hlsl",
		nullptr /*macros*/,
		nullptr /*include*/,
		"VS",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vertexShaderByteCode,
		&errorVertexCode);

	if (FAILED(res)) {
		// If the shader failed to compile it should have written something to the error message.
		if (errorVertexCode) {
			char* compileErrors = (char*)(errorVertexCode->GetBufferPointer());

			std::cout << compileErrors << std::endl;
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hWindow, L"ShadowMapShader.hlsl", L"Missing Shader File", MB_OK);
		}

		return 0;
	}

	ID3DBlob* errorGeometryCode = nullptr;

	res = D3DCompileFromFile(L"./Shaders/ShadowMapShader.hlsl",
		nullptr /*macros*/,
		nullptr /*include*/,
		"GS",
		"gs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&geometryShaderByteCode,
		&errorGeometryCode);

	if (FAILED(res)) {
		// If the shader failed to compile it should have written something to the error message.
		if (errorGeometryCode) {
			char* compileErrors = (char*)(errorGeometryCode->GetBufferPointer());

			std::cout << compileErrors << std::endl;
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hWindow, L"ShadowMapShader.hlsl", L"Missing Shader File", MB_OK);
		}

		return 0;
	}

	D3D11_INPUT_ELEMENT_DESC inputElements[] = {
		D3D11_INPUT_ELEMENT_DESC {
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0}
	};

	//create input layout

	device->CreateInputLayout(
		inputElements,
		sizeof(inputElements) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		vertexShaderByteCode->GetBufferPointer(),
		vertexShaderByteCode->GetBufferSize(),
		&layout);

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_FRONT;
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.DepthBias = 10000.0f;
	rastDesc.DepthBiasClamp = 0.0f;
	rastDesc.SlopeScaledDepthBias = 1.0f;

	res = device->CreateRasterizerState(&rastDesc, &rastState);

	//create vertex shader

	device->CreateVertexShader(
		vertexShaderByteCode->GetBufferPointer(),
		vertexShaderByteCode->GetBufferSize(),
		nullptr, &vertexShader);

	device->CreateGeometryShader(
		geometryShaderByteCode->GetBufferPointer(),
		geometryShaderByteCode->GetBufferSize(),
		nullptr, &geometryShader);

	return 1;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShadowMap::GetDepthMapSRV()
{
	return depthMapSRV;
}

void ShadowMap::BindDsvAndSetNullRenderTarget(ID3D11DeviceContext* context)
{
	context->RSSetViewports(1, &sViewport);

	context->RSSetState(rastState);

	context->IASetInputLayout(layout);
	context->VSSetShader(vertexShader, nullptr, 0);
	context->GSSetShader(geometryShader, nullptr, 0);
	context->PSSetShader(nullptr, nullptr, 0);

	ID3D11RenderTargetView* sRenderTargets[1] = { 0 };
	context->OMSetRenderTargets(1, sRenderTargets, depthMapDSV);
	context->ClearDepthStencilView(depthMapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
//END