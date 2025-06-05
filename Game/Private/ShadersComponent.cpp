#include "ShadersComponent.h"

ShadersComponent::ShadersComponent() {}

//compile and bind all shaders
int ShadersComponent::Initialize(HWND hWindow, Microsoft::WRL::ComPtr<ID3D11Device> device, ID3D11DeviceContext* context) {

	//create opaque vertex shader
	ID3DBlob* errorVertexCode = nullptr;

	auto res = D3DCompileFromFile(L"./Shaders/OpaqueShader.hlsl",
		nullptr /*macros*/,
		nullptr /*include*/,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&opaqueVertexShaderByteCode,
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
			MessageBox(hWindow, L"OpaqueShader.hlsl", L"Missing Shader File", MB_OK);
		}

		return 0;
	}

	device->CreateVertexShader(
		opaqueVertexShaderByteCode->GetBufferPointer(),
		opaqueVertexShaderByteCode->GetBufferSize(),
		nullptr, &opaqueVertexShader);

	//create opaque pixel shader
	D3D_SHADER_MACRO Shader_Macros[] = { "TEST", "1", "TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)", nullptr, nullptr };

	ID3DBlob* errorPixelCode = nullptr;

	res = D3DCompileFromFile(L"./Shaders/OpaqueShader.hlsl",
		Shader_Macros /*macros*/,
		nullptr /*include*/,
		"PSMain",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&opaquePixelShaderByteCode,
		&errorPixelCode);

	if (FAILED(res)) {
		// If the shader failed to compile it should have written something to the error message.
		if (errorPixelCode) {
			char* compileErrors = (char*)(errorPixelCode->GetBufferPointer());

			std::cout << compileErrors << std::endl;
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hWindow, L"OpaqueShader.hlsl", L"Missing Shader File", MB_OK);
		}

		return 0;
	}

	device->CreatePixelShader(
		opaquePixelShaderByteCode->GetBufferPointer(),
		opaquePixelShaderByteCode->GetBufferSize(),
		nullptr, &opaquePixelShader);

	//create opaque input layout
	D3D11_INPUT_ELEMENT_DESC opaqueInputElements[] = {
		D3D11_INPUT_ELEMENT_DESC {
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0},
		D3D11_INPUT_ELEMENT_DESC {
			"COLOR",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0},
		D3D11_INPUT_ELEMENT_DESC {
			"NORMAL",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0},
		D3D11_INPUT_ELEMENT_DESC {
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0}
	};

	device->CreateInputLayout(
		opaqueInputElements,
		sizeof(opaqueInputElements) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		opaqueVertexShaderByteCode->GetBufferPointer(),
		opaqueVertexShaderByteCode->GetBufferSize(),
		&opaqueLayout);


	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_FRONT;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	res = device->CreateRasterizerState(&rastDesc, &rastState);

	//create texture sampler
	CD3D11_SAMPLER_DESC textureSamplerDesc = {};
	textureSamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	textureSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	textureSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	textureSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	textureSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	textureSamplerDesc.MaxAnisotropy = 16;

	res = device->CreateSamplerState(&textureSamplerDesc, &textureSampler);
	//end creating opaque shaders

	//create lighting vertex shader
	res = D3DCompileFromFile(L"./Shaders/LightingShader.hlsl",
		nullptr /*macros*/,
		nullptr /*include*/,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&lightingVertexShaderByteCode,
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
			MessageBox(hWindow, L"LightingShader.hlsl", L"Missing Shader File", MB_OK);
		}

		return 0;
	}

	device->CreateVertexShader(
		lightingVertexShaderByteCode->GetBufferPointer(),
		lightingVertexShaderByteCode->GetBufferSize(),
		nullptr, &lightingVertexShader);

	//create lighting pixel shader
	res = D3DCompileFromFile(L"./Shaders/LightingShader.hlsl",
		Shader_Macros /*macros*/,
		nullptr /*include*/,
		"PSMain",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&lightingPixelShaderByteCode,
		&errorPixelCode);

	if (FAILED(res)) {
		// If the shader failed to compile it should have written something to the error message.
		if (errorPixelCode) {
			char* compileErrors = (char*)(errorPixelCode->GetBufferPointer());

			std::cout << compileErrors << std::endl;
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hWindow, L"LightingShader.hlsl", L"Missing Shader File", MB_OK);
		}

		return 0;
	}

	device->CreatePixelShader(
		lightingPixelShaderByteCode->GetBufferPointer(),
		lightingPixelShaderByteCode->GetBufferSize(),
		nullptr, &lightingPixelShader);

	//create lighting input layout
	D3D11_INPUT_ELEMENT_DESC lightingInputElements[] = {
		D3D11_INPUT_ELEMENT_DESC {
			"POSITION",
			0,
			DXGI_FORMAT_R32_UINT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0}
	};

	device->CreateInputLayout(
		lightingInputElements,
		sizeof(lightingInputElements) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		lightingVertexShaderByteCode->GetBufferPointer(),
		lightingVertexShaderByteCode->GetBufferSize(),
		&lightingLayout);

	//create shadow sampler
	CD3D11_SAMPLER_DESC shadowSamplerDesc = {};
	shadowSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

	res = device->CreateSamplerState(&shadowSamplerDesc, &shadowSampler);

	//create transparent vertex shader
	res = D3DCompileFromFile(L"./Shaders/MyVeryFirstShader.hlsl",
		nullptr /*macros*/,
		nullptr /*include*/,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&transparentVertexShaderByteCode,
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
			MessageBox(hWindow, L"LightingShader.hlsl", L"Missing Shader File", MB_OK);
		}

		return 0;
	}

	device->CreateVertexShader(
		transparentVertexShaderByteCode->GetBufferPointer(),
		transparentVertexShaderByteCode->GetBufferSize(),
		nullptr, &transparentVertexShader);

	//create transparent pixel shader
	res = D3DCompileFromFile(L"./Shaders/MyVeryFirstShader.hlsl",
		Shader_Macros /*macros*/,
		nullptr /*include*/,
		"PSMain",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&transparentPixelShaderByteCode,
		&errorPixelCode);

	if (FAILED(res)) {
		// If the shader failed to compile it should have written something to the error message.
		if (errorPixelCode) {
			char* compileErrors = (char*)(errorPixelCode->GetBufferPointer());

			std::cout << compileErrors << std::endl;
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hWindow, L"LightingShader.hlsl", L"Missing Shader File", MB_OK);
		}

		return 0;
	}

	device->CreatePixelShader(
		transparentPixelShaderByteCode->GetBufferPointer(),
		transparentPixelShaderByteCode->GetBufferSize(),
		nullptr, &transparentPixelShader);

	//create lighting input layout
	D3D11_INPUT_ELEMENT_DESC transparentInputElements[] = {
		D3D11_INPUT_ELEMENT_DESC {
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0},
		D3D11_INPUT_ELEMENT_DESC {
			"COLOR",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0},
		D3D11_INPUT_ELEMENT_DESC {
			"NORMAL",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0},
		D3D11_INPUT_ELEMENT_DESC {
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0}
	};

	device->CreateInputLayout(
		transparentInputElements,
		sizeof(transparentInputElements) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		transparentVertexShaderByteCode->GetBufferPointer(),
		transparentVertexShaderByteCode->GetBufferSize(),
		&transparentLayout);

	//create particles vertex shader
	res = D3DCompileFromFile(L"./Shaders/FountainShader.hlsl",
		nullptr /*macros*/,
		nullptr /*include*/,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&particlesVertexShaderByteCode,
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
			MessageBox(hWindow, L"FountainShader.hlsl", L"Missing Shader File", MB_OK);
		}

		return 0;
	}

	device->CreateVertexShader(
		particlesVertexShaderByteCode->GetBufferPointer(),
		particlesVertexShaderByteCode->GetBufferSize(),
		nullptr, &particlesVertexShader);

	//create particles pixel shader
	res = D3DCompileFromFile(L"./Shaders/FountainShader.hlsl",
		Shader_Macros /*macros*/,
		nullptr /*include*/,
		"PSMain",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&particlesPixelShaderByteCode,
		&errorPixelCode);

	if (FAILED(res)) {
		// If the shader failed to compile it should have written something to the error message.
		if (errorPixelCode) {
			char* compileErrors = (char*)(errorPixelCode->GetBufferPointer());

			std::cout << compileErrors << std::endl;
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hWindow, L"FountainShader.hlsl", L"Missing Shader File", MB_OK);
		}

		return 0;
	}

	device->CreatePixelShader(
		particlesPixelShaderByteCode->GetBufferPointer(),
		particlesPixelShaderByteCode->GetBufferSize(),
		nullptr, &particlesPixelShader);

	ID3DBlob* errorComputeCode = nullptr;

	//create particles compute shader
	res = D3DCompileFromFile(L"./Shaders/ParticleComputeShaders.hlsl",
		Shader_Macros /*macros*/,
		nullptr /*include*/,
		"CSInit",
		"cs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&particlesInitShaderByteCode,
		&errorComputeCode);

	if (FAILED(res)) {
		// If the shader failed to compile it should have written something to the error message.
		if (errorComputeCode) {
			char* compileErrors = (char*)(errorComputeCode->GetBufferPointer());

			std::cout << compileErrors << std::endl;
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hWindow, L"FountainShader.hlsl", L"Missing Shader File", MB_OK);
		}

		return 0;
	}

	device->CreateComputeShader(
		particlesInitShaderByteCode->GetBufferPointer(),
		particlesInitShaderByteCode->GetBufferSize(),
		nullptr, &particlesInitShader);

	//create particles emit shader
	res = D3DCompileFromFile(L"./Shaders/ParticleComputeShaders.hlsl",
		Shader_Macros /*macros*/,
		nullptr /*include*/,
		"CSEmit",
		"cs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&particlesEmitShaderByteCode,
		&errorComputeCode);

	if (FAILED(res)) {
		// If the shader failed to compile it should have written something to the error message.
		if (errorComputeCode) {
			char* compileErrors = (char*)(errorComputeCode->GetBufferPointer());

			std::cout << compileErrors << std::endl;
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hWindow, L"FountainShader.hlsl", L"Missing Shader File", MB_OK);
		}

		return 0;
	}

	device->CreateComputeShader(
		particlesEmitShaderByteCode->GetBufferPointer(),
		particlesEmitShaderByteCode->GetBufferSize(),
		nullptr, &particlesEmitShader);

	//create particles update shader
	res = D3DCompileFromFile(L"./Shaders/ParticleComputeShaders.hlsl",
		Shader_Macros /*macros*/,
		nullptr /*include*/,
		"CSUpdate",
		"cs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&particlesUpdateShaderByteCode,
		&errorComputeCode);

	if (FAILED(res)) {
		// If the shader failed to compile it should have written something to the error message.
		if (errorComputeCode) {
			char* compileErrors = (char*)(errorComputeCode->GetBufferPointer());

			std::cout << compileErrors << std::endl;
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hWindow, L"FountainShader.hlsl", L"Missing Shader File", MB_OK);
		}

		return 0;
	}

	device->CreateComputeShader(
		particlesUpdateShaderByteCode->GetBufferPointer(),
		particlesUpdateShaderByteCode->GetBufferSize(),
		nullptr, &particlesUpdateShader);

	//create particles update shader
	res = D3DCompileFromFile(L"./Shaders/ParticleComputeShaders.hlsl",
		Shader_Macros /*macros*/,
		nullptr /*include*/,
		"CSConsume",
		"cs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&particlesConsumeShaderByteCode,
		&errorComputeCode);

	if (FAILED(res)) {
		// If the shader failed to compile it should have written something to the error message.
		if (errorComputeCode) {
			char* compileErrors = (char*)(errorComputeCode->GetBufferPointer());

			std::cout << compileErrors << std::endl;
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hWindow, L"FountainShader.hlsl", L"Missing Shader File", MB_OK);
		}

		return 0;
	}

	device->CreateComputeShader(
		particlesConsumeShaderByteCode->GetBufferPointer(),
		particlesConsumeShaderByteCode->GetBufferSize(),
		nullptr, &particlesConsumeShader);


	//create particles compute sort shader
	res = D3DCompileFromFile(L"./Shaders/ComputeShaderSort.hlsl",
		Shader_Macros /*macros*/,
		nullptr /*include*/,
		"BitonicSort",
		"cs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&particlesComputeSortShaderByteCode,
		&errorComputeCode);

	if (FAILED(res)) {
		// If the shader failed to compile it should have written something to the error message.
		if (errorComputeCode) {
			char* compileErrors = (char*)(errorComputeCode->GetBufferPointer());

			std::cout << compileErrors << std::endl;
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hWindow, L"ComputeShaderSort.hlsl", L"Missing Shader File", MB_OK);
		}

		return 0;
	}

	device->CreateComputeShader(
		particlesComputeSortShaderByteCode->GetBufferPointer(),
		particlesComputeSortShaderByteCode->GetBufferSize(),
		nullptr, &particlesComputeSortShader);


	//create particles compute transpose shader
	res = D3DCompileFromFile(L"./Shaders/ComputeShaderSort.hlsl",
		Shader_Macros /*macros*/,
		nullptr /*include*/,
		"MatrixTranspose",
		"cs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&particlesComputeTransposeShaderByteCode,
		&errorComputeCode);

	if (FAILED(res)) {
		// If the shader failed to compile it should have written something to the error message.
		if (errorComputeCode) {
			char* compileErrors = (char*)(errorComputeCode->GetBufferPointer());

			std::cout << compileErrors << std::endl;
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hWindow, L"ComputeShaderSort.hlsl", L"Missing Shader File", MB_OK);
		}

		return 0;
	}

	device->CreateComputeShader(
		particlesComputeTransposeShaderByteCode->GetBufferPointer(),
		particlesComputeTransposeShaderByteCode->GetBufferSize(),
		nullptr, &particlesComputeTransposeShader);


	//create lighting input layout
	D3D11_INPUT_ELEMENT_DESC particlesInputElements[] = {
		D3D11_INPUT_ELEMENT_DESC {
			"INDEX",
			0,
			DXGI_FORMAT_R32_UINT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0}
	};

	device->CreateInputLayout(
		particlesInputElements,
		sizeof(particlesInputElements) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		particlesVertexShaderByteCode->GetBufferPointer(),
		particlesVertexShaderByteCode->GetBufferSize(),
		&particlesLayout);

	return 1;
}

void ShadersComponent::DrawOpaque(ID3D11DeviceContext* context)
{
	context->RSSetState(rastState);

	context->IASetInputLayout(opaqueLayout);
	context->VSSetShader(opaqueVertexShader, nullptr, 0);
	context->PSSetShader(opaquePixelShader, nullptr, 0);
	context->PSSetSamplers(0, 1, &textureSampler);

}

void ShadersComponent::DrawLighting(ID3D11DeviceContext* context)
{

	context->RSSetState(rastState);

	context->IASetInputLayout(lightingLayout);
	context->VSSetShader(lightingVertexShader, nullptr, 0);
	context->PSSetShader(lightingPixelShader, nullptr, 0);
	context->PSSetSamplers(0, 1, &textureSampler);
	context->PSSetSamplers(1, 1, &shadowSampler);
}

void ShadersComponent::DrawTransparent(ID3D11DeviceContext* context)
{

	context->RSSetState(rastState);

	context->IASetInputLayout(transparentLayout);
	context->VSSetShader(transparentVertexShader, nullptr, 0);
	context->PSSetShader(transparentPixelShader, nullptr, 0);
	context->PSSetSamplers(0, 1, &textureSampler);
	context->PSSetSamplers(1, 1, &shadowSampler);
}

void ShadersComponent::InitParticleSystems(ID3D11DeviceContext* context)
{
	context->CSSetShader(particlesInitShader, nullptr, 0);

	context->Dispatch(32, 24, 1);

	ID3D11UnorderedAccessView* pViewnullptr = nullptr;
	context->CSSetUnorderedAccessViews(2, 1, &pViewnullptr, nullptr);
}

void ShadersComponent::EmitParticles(ID3D11DeviceContext* context)
{
	context->CSSetShader(particlesEmitShader, nullptr, 0);

	context->Dispatch(32, 24, 1);

	ID3D11UnorderedAccessView* pViewnullptr = nullptr;
	context->CSSetUnorderedAccessViews(1, 1, &pViewnullptr, nullptr);
	context->CSSetUnorderedAccessViews(3, 1, &pViewnullptr, nullptr);
}

void ShadersComponent::UpdateParticles(ID3D11DeviceContext* context)
{
	context->CSSetShader(particlesUpdateShader, nullptr, 0);

	context->Dispatch(32, 24, 1);

	ID3D11UnorderedAccessView* pViewnullptr = nullptr;
	context->CSSetUnorderedAccessViews(0, 1, &pViewnullptr, nullptr);
	context->CSSetUnorderedAccessViews(2, 1, &pViewnullptr, nullptr);
}

void ShadersComponent::ConsumeParticles(ID3D11DeviceContext* context)
{
	context->CSSetShader(particlesConsumeShader, nullptr, 0);

	context->Dispatch(32, 24, 1);

	ID3D11UnorderedAccessView* pViewnullptr = nullptr;
	context->CSSetUnorderedAccessViews(2, 1, &pViewnullptr, nullptr);
	context->CSSetUnorderedAccessViews(3, 1, &pViewnullptr, nullptr);
}

void ShadersComponent::DrawParticles(ID3D11DeviceContext* context)
{
	context->RSSetState(rastState);

	//context->IASetInputLayout(particlesLayout);
	context->VSSetShader(particlesVertexShader, nullptr, 0);
	context->PSSetShader(particlesPixelShader, nullptr, 0);
}

void ShadersComponent::SortParticles(ID3D11DeviceContext* context)
{
	context->CSSetShader(particlesComputeSortShader, nullptr, 0);
}

void ShadersComponent::TransposeParticles(ID3D11DeviceContext* context)
{
	context->CSSetShader(particlesComputeTransposeShader, nullptr, 0);
}
