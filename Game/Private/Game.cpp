#include "Game.h"

Game* Game::instance = nullptr;

Game::Game(HINSTANCE hInst, LPCWSTR appName) : hInstance(hInst), applicationName(appName) {

	clientWidth = 800;
	clientHeight = 800;

	display = new DisplayWin32(clientWidth, clientHeight);
	instance = this;
}

void Game::Initialize(UINT objCnt, UINT lightsCnt, UINT mapSize) {

	input = new InputDevice(this);
	hWindow = display->Init(hInstance, applicationName);

	
	D3D_FEATURE_LEVEL featureLevel[] = { D3D_FEATURE_LEVEL_11_1 };

	swapDescriptor.BufferDesc.Width = clientWidth;
	swapDescriptor.BufferDesc.Height = clientHeight;
	swapDescriptor.BufferDesc.RefreshRate.Numerator = 60;
	swapDescriptor.BufferDesc.RefreshRate.Denominator = 1;
	swapDescriptor.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDescriptor.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDescriptor.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapDescriptor.SampleDesc.Count = 1;
	swapDescriptor.SampleDesc.Quality = 0;
	swapDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDescriptor.BufferCount = 2;
	swapDescriptor.OutputWindow = hWindow;
	swapDescriptor.Windowed = true;
	swapDescriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swapDescriptor.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	auto res = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_DEBUG,
		featureLevel,
		1,
		D3D11_SDK_VERSION,
		&swapDescriptor,
		&swapChain,
		&device,
		nullptr,
		&context);

	if (FAILED(res))
	{
		// Well, that was unexpected
	}

	shaders = new ShadersComponent();

	shaders->Initialize(hWindow, device, context);

	camManager = new CameraManager();

	//Lights init
	lightBufData = new LightningData();
	lightBufData->lightsNum = lightsCnt;
	//

	//shadow maps init
	//+1 is for directional light
	shadowMapProperties = new shadowMapProps();

	int ShadowMapSize = 8192;

	for (int i = 0; i < /*lightBufData->lightsNum +*/ 1; ++i)
	{
		shadowMaps.push_back(new ShadowMap(device, ShadowMapSize, ShadowMapSize));
		shadowMaps[i]->ShadersInitialize(hWindow, device, context);
	}
	//END

	//Transparent objects array init
	this->mapSize = mapSize;

	for (int i = 0; i < 100; ++i)
		transpObjects.push_back(std::vector<GameComponent*>(mapSize * 2));
	//
}

void Game::PrepareResources() {
	HRESULT res = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	//res = device->CreateRenderTargetView(backBuffer, nullptr, &renderView); // second parameter may be not nullptr if it's not for backbuffer

	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = clientWidth;
	textureDesc.Height = clientHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET;

	res = device->CreateRenderTargetView(backBuffer, nullptr, &opaqueRenderView[0]); // second parameter may be not nullptr if it's not for backbuffer
	for (int i = 0; i < 4; ++i)
	{
		res = device->CreateTexture2D(&textureDesc, nullptr, &opaqueBuffer[i]);
		res = device->CreateRenderTargetView(opaqueBuffer[i], nullptr, &opaqueRenderView[i+1]); // second parameter may be not nullptr if it's not for backbuffer
	}

	//create depth/stencil buffer and view

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = clientWidth;
	depthStencilDesc.Height = clientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	res = device->CreateTexture2D(&depthStencilDesc, 0, &depthStencilBuffer);
	res = device->CreateDepthStencilView(depthStencilBuffer, 0, &depthStencilView);
	context->OMSetRenderTargets(1, &renderView, depthStencilView);

	D3D11_BLEND_DESC blendDesc;

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	res = device->CreateBlendState(&blendDesc, &blendState);

	D3D11_BUFFER_DESC lightBufDesc = {};
	lightBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufDesc.MiscFlags = 0;
	lightBufDesc.StructureByteStride = 0;
	lightBufDesc.ByteWidth = sizeof(LightningData);

	res = device->CreateBuffer(&lightBufDesc, 0, &lightBuf);

	D3D11_BUFFER_DESC cascadeShadowBufDesc = {};
	cascadeShadowBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	cascadeShadowBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cascadeShadowBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cascadeShadowBufDesc.MiscFlags = 0;
	cascadeShadowBufDesc.StructureByteStride = 0;
	cascadeShadowBufDesc.ByteWidth = sizeof(shadowMapProps);

	HRESULT hr = device->CreateBuffer(&cascadeShadowBufDesc, 0, &cascadeShadowPropsBuffer);
}

void Game::Run() {
	//timer.Reset();
	PrevTime = std::chrono::steady_clock::now();
	totalTime = 0;
	frameCount = 0;

	MSG msg = {};
	bool isExitRequested = false;

	while (!isExitRequested) {

		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If windows signals to end the application then exit out.
		if (MessageHandler(msg.message)) {
			isExitRequested = 1;
			continue;
		}

		//timer.Tick();

		auto	curTime = std::chrono::steady_clock::now();
		float	deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(curTime - PrevTime).count() / 1000000.0f;
		PrevTime = curTime;

		Update(deltaTime);
		
		//SHADOWS
		BuildShadowTransform();

		context->ClearState();

		shadowMaps[0]->BindDsvAndSetNullRenderTarget(context);

		DrawSceneToShadowMap();
		//END

		context->ClearState();

		PrepareFrame();

		RestoreTargets(5, &opaqueRenderView[0], depthStencilView);

		Draw();

		swapChain->Present(1, /*DXGI_PRESENT_DO_NOT_WAIT*/ 0);

		RestoreTargets();

		//context->ClearState();

		//PrepareFrame();

		//RestoreTargets(1, &opaqueRenderView[0], depthStencilView);

		//Draw();

		//swapChain->Present(1, /*DXGI_PRESENT_DO_NOT_WAIT*/ 0);

		//RestoreTargets();
	}
}

int Game::MessageHandler(UINT msg) {
	switch (msg) {
	case WM_QUIT:
		return 1;
	}
	return 0;
}

void Game::PrepareFrame() {

	D3D11_VIEWPORT* viewport = new D3D11_VIEWPORT();
	viewport->Width = (float) clientWidth;
	viewport->Height = (float) clientHeight;
	viewport->TopLeftX = 0;
	viewport->TopLeftY = 0;
	viewport->MinDepth = 0;
	viewport->MaxDepth = 1.0f; 
	context->RSSetViewports(1, viewport);

	if (lightBufData != nullptr) {
		D3D11_MAPPED_SUBRESOURCE res = {};
		context->PSSetConstantBuffers(1, 1, &lightBuf);
		context->Map(lightBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);


		auto dataPtr = reinterpret_cast<float*>(res.pData);
		memcpy(dataPtr, lightBufData, sizeof(LightningData));

		context->Unmap(lightBuf, 0);
	}

	if (shadowMaps[0] != nullptr)
	{
		context->PSSetShaderResources(1, 1, shadowMaps[0]->GetDepthMapSRV().GetAddressOf());
	}
}

void Game::Update(float deltaTime) {


	totalTime += deltaTime;
	frameCount++;

	if (totalTime > 1.0f) {
		float fps = frameCount / totalTime;

		totalTime -= 1.0f;

		WCHAR text[256];
		swprintf_s(text, TEXT("FPS: %f"), fps);
		SetWindowText(swapDescriptor.OutputWindow, text);

		frameCount = 0;
	}

}

void Game::Draw() {

	float color[] = { 0.1f, 0.1f, 0.2f, 1.0f };
	context->ClearRenderTargetView(opaqueRenderView[0], color);
	for (int i = 1; i < 5; ++i)
	context->ClearRenderTargetView(opaqueRenderView[i], color);
	shaders->Draw(context);

	float blendFactors[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->OMSetBlendState(blendState, blendFactors, 0xffffffff);

	/*for (auto object : objects)
		object->Update(camManager);*/

	D3D11_MAPPED_SUBRESOURCE res = {};
	context->PSSetConstantBuffers(2, 1, &cascadeShadowPropsBuffer);
	context->Map(cascadeShadowPropsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);


	auto dataPtr = reinterpret_cast<float*>(res.pData);
	memcpy(dataPtr, shadowMapProperties, sizeof(shadowMapProps));

	context->Unmap(cascadeShadowPropsBuffer, 0);

	for (GameComponent* object : objects) {
		if (!object->isTransparent)
			object->Draw(context);
	}
	DrawTransparent();
}


void Game::DrawTransparent()
{
	for (GameComponent* object : objects)
	{
		if (object->isTransparent)
		{
			Matrix transformMat = object->properties->transformW.Transpose();
			transformMat *= object->properties->transformH.Transpose();

			for (int i = 0; i < 100; ++i)
			{
				int k = (int)abs(transformMat._43);
				if (transpObjects[i][k] == NULL) {
					transpObjects[i][k] = object;
					break;
				}
			}
		}
	}
	for (int i = mapSize - 1; i >= 0; --i)
	{
		for (int j = 0; j < 100; ++j)
		{
			if (transpObjects[j][i] == NULL)
			{
				break;
			}
			else
			{
				transpObjects[j][i]->Draw(context);
				transpObjects[j][i] = NULL;
			}
		}
	}
}

//SHADOWS
void Game::DrawSceneToShadowMap()
{
	//float color[] = { 0.1, 0.1f, 0.2f, 1.0f };
	//context->ClearRenderTargetView(renderView, color);

	D3D11_MAPPED_SUBRESOURCE res = {};
	context->GSSetConstantBuffers(1, 1, &cascadeShadowPropsBuffer);
	context->Map(cascadeShadowPropsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);


	auto dataPtr = reinterpret_cast<float*>(res.pData);
	memcpy(dataPtr, shadowMapProperties, sizeof(shadowMapProps));

	context->Unmap(cascadeShadowPropsBuffer, 0);

	for (auto object : objects) {
		object->DrawShadow(context);
	}
}
//END

void Game::RestoreTargets(int viewsCnt, ID3D11RenderTargetView* const* RenderView, ID3D11DepthStencilView* DepthStencilView) {
	context->OMSetRenderTargets(viewsCnt, RenderView, DepthStencilView);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1, 1);
}

void Game::DestroyResources() {

}

int Game::Exit() {

	DestroyResources();

	return 1;
}

const HWND& Game::MainWindow() {
	return hWindow;
}

void Game::BuildShadowTransform()
{

	Matrix T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	float radius = sceneBounds.radius;
	Vector3 center = sceneBounds.center;

	//for (int i = 0; i < CascadesCount; --i)
	for (int i = CascadesCount - 1; i >= 0; --i)
	{
		Vector3 lightDir = Vector3(lightBufData->dirLight.direction);
		//lightDir.Normalize(lightDir);
		Vector3 lightPos = radius * lightDir + center;
		Vector3 targetPos = center;

		Matrix V = Matrix::CreateLookAt(lightPos, targetPos, Vector3::Up);

		Vector3 sceneCenterLS = Vector3::Transform(targetPos, V);

		float l = sceneCenterLS.x - radius * 2.0f;
		float b = sceneCenterLS.y - radius * 2.0f;
		float n = sceneCenterLS.z - radius * 2.0f;
		float r = sceneCenterLS.x + radius * 2.0f;
		float t = sceneCenterLS.y + radius * 2.0f;
		float f = sceneCenterLS.z + radius * 2.0f;

		Matrix P = DirectX::XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

		Matrix S = V * P * T;

		radius /= 2.5f;

		(V * P).Transpose(shadowMapProperties->transformVP[i]); //TODO:  CascadesCount - 1

		S.Transpose(shadowMapProperties->transformS[i]);

		float* distance = &shadowMapProperties->distances->x;
		*(distance + i) = f * 2.0f * cos(lightDir.y);
	}
}

//#include "Game.h"
//
//Game* Game::instance = nullptr;
//
//Game::Game(HINSTANCE hInst, LPCWSTR appName) : hInstance(hInst), applicationName(appName) {
//
//	clientWidth = 800;
//	clientHeight = 800;
//
//	display = new DisplayWin32(clientWidth, clientHeight);
//	instance = this;
//}
//
//void Game::Initialize(UINT objCnt, UINT lightsCnt, UINT mapSize) {
//
//	input = new InputDevice(this);
//	hWindow = display->Init(hInstance, applicationName);
//
//	
//	D3D_FEATURE_LEVEL featureLevel[] = { D3D_FEATURE_LEVEL_11_1 };
//
//	swapDescriptor.BufferDesc.Width = clientWidth;
//	swapDescriptor.BufferDesc.Height = clientHeight;
//	swapDescriptor.BufferDesc.RefreshRate.Numerator = 60;
//	swapDescriptor.BufferDesc.RefreshRate.Denominator = 1;
//	swapDescriptor.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//	swapDescriptor.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
//	swapDescriptor.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
//	swapDescriptor.SampleDesc.Count = 1;
//	swapDescriptor.SampleDesc.Quality = 0;
//	swapDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//	swapDescriptor.BufferCount = 3;
//	swapDescriptor.OutputWindow = hWindow;
//	swapDescriptor.Windowed = true;
//	swapDescriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
//	swapDescriptor.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
//
//	auto res = D3D11CreateDeviceAndSwapChain(
//		nullptr,
//		D3D_DRIVER_TYPE_HARDWARE,
//		nullptr,
//		D3D11_CREATE_DEVICE_DEBUG,
//		featureLevel,
//		1,
//		D3D11_SDK_VERSION,
//		&swapDescriptor,
//		&swapChain,
//		&device,
//		nullptr,
//		&context);
//
//	if (FAILED(res))
//	{
//		// Well, that was unexpected
//	}
//
//	shaders = new ShadersComponent();
//
//	shaders->Initialize(hWindow, device, context);
//
//	camManager = new CameraManager();
//
//	//Lights init
//	lightBufData = new LightningData();
//	lightBufData->lightsNum = lightsCnt;
//	//
//
//	//shadow maps init
//	//+1 is for directional light
//	shadowMapProperties = new shadowMapProps();
//
//	int ShadowMapSize = 8192;
//
//	for (int i = 0; i < /*lightBufData->lightsNum +*/ 1; ++i)
//	{
//		shadowMaps.push_back(new ShadowMap(device, ShadowMapSize, ShadowMapSize));
//		shadowMaps[i]->ShadersInitialize(hWindow, device, context);
//	}
//	//END
//
//	//Transparent objects array init
//	this->mapSize = mapSize;
//
//	for (int i = 0; i < 100; ++i)
//		transpObjects.push_back(std::vector<GameComponent*>(mapSize * 2));
//	//
//}
//
//void Game::PrepareResources() {
//	HRESULT res = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
//	res = device->CreateRenderTargetView(backBuffer, nullptr, &renderView); // second parameter may be not nullptr if it's not for backbuffer
//
//	//create depth/stencil buffer and view
//
//	D3D11_TEXTURE2D_DESC depthStencilDesc;
//
//	depthStencilDesc.Width = clientWidth;
//	depthStencilDesc.Height = clientHeight;
//	depthStencilDesc.MipLevels = 1;
//	depthStencilDesc.ArraySize = 1;
//	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
//	depthStencilDesc.SampleDesc.Count = 1;
//	depthStencilDesc.SampleDesc.Quality = 0;
//	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
//	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
//	depthStencilDesc.CPUAccessFlags = 0;
//	depthStencilDesc.MiscFlags = 0;
//
//	res = device->CreateTexture2D(&depthStencilDesc, 0, &depthStencilBuffer);
//	res = device->CreateDepthStencilView(depthStencilBuffer, 0, &depthStencilView);
//	context->OMSetRenderTargets(1, &renderView, depthStencilView);
//
//	D3D11_BLEND_DESC blendDesc;
//
//	blendDesc.AlphaToCoverageEnable = false;
//	blendDesc.IndependentBlendEnable = false;
//	blendDesc.RenderTarget[0].BlendEnable = true;
//	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
//	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
//	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
//	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
//	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
//	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
//	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
//
//	res = device->CreateBlendState(&blendDesc, &blendState);
//
//	D3D11_BUFFER_DESC lightBufDesc = {};
//	lightBufDesc.Usage = D3D11_USAGE_DYNAMIC;
//	lightBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//	lightBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//	lightBufDesc.MiscFlags = 0;
//	lightBufDesc.StructureByteStride = 0;
//	lightBufDesc.ByteWidth = sizeof(LightningData);
//
//	res = device->CreateBuffer(&lightBufDesc, 0, &lightBuf);
//
//	D3D11_BUFFER_DESC cascadeShadowBufDesc = {};
//	cascadeShadowBufDesc.Usage = D3D11_USAGE_DYNAMIC;
//	cascadeShadowBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//	cascadeShadowBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//	cascadeShadowBufDesc.MiscFlags = 0;
//	cascadeShadowBufDesc.StructureByteStride = 0;
//	cascadeShadowBufDesc.ByteWidth = sizeof(shadowMapProps);
//
//	HRESULT hr = device->CreateBuffer(&cascadeShadowBufDesc, 0, &cascadeShadowPropsBuffer);
//}
//
//void Game::Run() {
//	//timer.Reset();
//	PrevTime = std::chrono::steady_clock::now();
//	totalTime = 0;
//	frameCount = 0;
//
//	MSG msg = {};
//	bool isExitRequested = false;
//
//	while (!isExitRequested) {
//
//		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
//			TranslateMessage(&msg);
//			DispatchMessage(&msg);
//		}
//
//		// If windows signals to end the application then exit out.
//		if (MessageHandler(msg.message)) {
//			isExitRequested = 1;
//			continue;
//		}
//
//		//timer.Tick();
//
//		auto	curTime = std::chrono::steady_clock::now();
//		float	deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(curTime - PrevTime).count() / 1000000.0f;
//		PrevTime = curTime;
//
//		Update(deltaTime);
//		
//		//SHADOWS
//		BuildShadowTransform();
//
//		context->ClearState();
//
//		shadowMaps[0]->BindDsvAndSetNullRenderTarget(context);
//
//		DrawSceneToShadowMap();
//		//END
//
//		context->ClearState();
//
//		PrepareFrame();
//
//		RestoreTargets(1, &renderView, depthStencilView);
//
//		Draw();
//
//		swapChain->Present(1, /*DXGI_PRESENT_DO_NOT_WAIT*/ 0);
//
//		RestoreTargets();
//	}
//}
//
//int Game::MessageHandler(UINT msg) {
//	switch (msg) {
//	case WM_QUIT:
//		return 1;
//	}
//	return 0;
//}
//
//void Game::PrepareFrame() {
//
//	D3D11_VIEWPORT* viewport = new D3D11_VIEWPORT();
//	viewport->Width = (float) clientWidth;
//	viewport->Height = (float) clientHeight;
//	viewport->TopLeftX = 0;
//	viewport->TopLeftY = 0;
//	viewport->MinDepth = 0;
//	viewport->MaxDepth = 1.0f; 
//	context->RSSetViewports(1, viewport);
//
//	if (lightBufData != nullptr) {
//		D3D11_MAPPED_SUBRESOURCE res = {};
//		context->PSSetConstantBuffers(1, 1, &lightBuf);
//		context->Map(lightBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
//
//
//		auto dataPtr = reinterpret_cast<float*>(res.pData);
//		memcpy(dataPtr, lightBufData, sizeof(LightningData));
//
//		context->Unmap(lightBuf, 0);
//	}
//
//	if (shadowMaps[0] != nullptr)
//	{
//		context->PSSetShaderResources(1, 1, shadowMaps[0]->GetDepthMapSRV().GetAddressOf());
//	}
//}
//
//void Game::Update(float deltaTime) {
//
//
//	totalTime += deltaTime;
//	frameCount++;
//
//	if (totalTime > 1.0f) {
//		float fps = frameCount / totalTime;
//
//		totalTime -= 1.0f;
//
//		WCHAR text[256];
//		swprintf_s(text, TEXT("FPS: %f"), fps);
//		SetWindowText(swapDescriptor.OutputWindow, text);
//
//		frameCount = 0;
//	}
//
//}
//
//void Game::Draw() {
//
//	float color[] = { 0.1f, 0.1f, 0.2f, 1.0f };
//	context->ClearRenderTargetView(renderView, color);
//	shaders->Draw(context);
//
//	float blendFactors[] = { 0.0f, 0.0f, 0.0f, 0.0f };
//	context->OMSetBlendState(blendState, blendFactors, 0xffffffff);
//
//	/*for (auto object : objects)
//		object->Update(camManager);*/
//
//	D3D11_MAPPED_SUBRESOURCE res = {};
//	context->PSSetConstantBuffers(2, 1, &cascadeShadowPropsBuffer);
//	context->Map(cascadeShadowPropsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
//
//
//	auto dataPtr = reinterpret_cast<float*>(res.pData);
//	memcpy(dataPtr, shadowMapProperties, sizeof(shadowMapProps));
//
//	context->Unmap(cascadeShadowPropsBuffer, 0);
//
//	for (GameComponent* object : objects) {
//		if (!object->isTransparent)
//			object->Draw(context);
//	}
//	DrawTransparent();
//}
//
//
//void Game::DrawTransparent()
//{
//	for (GameComponent* object : objects)
//	{
//		if (object->isTransparent)
//		{
//			Matrix transformMat = object->properties->transformW.Transpose();
//			transformMat *= object->properties->transformH.Transpose();
//
//			for (int i = 0; i < 100; ++i)
//			{
//				int k = (int)abs(transformMat._43);
//				if (transpObjects[i][k] == NULL) {
//					transpObjects[i][k] = object;
//					break;
//				}
//			}
//		}
//	}
//	for (int i = mapSize - 1; i >= 0; --i)
//	{
//		for (int j = 0; j < 100; ++j)
//		{
//			if (transpObjects[j][i] == NULL)
//			{
//				break;
//			}
//			else
//			{
//				transpObjects[j][i]->Draw(context);
//				transpObjects[j][i] = NULL;
//			}
//		}
//	}
//}
//
////SHADOWS
//void Game::DrawSceneToShadowMap()
//{
//	//float color[] = { 0.1, 0.1f, 0.2f, 1.0f };
//	//context->ClearRenderTargetView(renderView, color);
//
//	D3D11_MAPPED_SUBRESOURCE res = {};
//	context->GSSetConstantBuffers(1, 1, &cascadeShadowPropsBuffer);
//	context->Map(cascadeShadowPropsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
//
//
//	auto dataPtr = reinterpret_cast<float*>(res.pData);
//	memcpy(dataPtr, shadowMapProperties, sizeof(shadowMapProps));
//
//	context->Unmap(cascadeShadowPropsBuffer, 0);
//
//	for (auto object : objects) {
//		object->DrawShadow(context);
//	}
//}
////END
//
//void Game::RestoreTargets(int viewsCnt, ID3D11RenderTargetView* const* RenderView, ID3D11DepthStencilView* DepthStencilView) {
//	context->OMSetRenderTargets(viewsCnt, RenderView, DepthStencilView);
//	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1, 1);
//}
//
//void Game::DestroyResources() {
//
//}
//
//int Game::Exit() {
//
//	DestroyResources();
//
//	return 1;
//}
//
//const HWND& Game::MainWindow() {
//	return hWindow;
//}
//
//void Game::BuildShadowTransform()
//{
//
//	Matrix T(
//		0.5f, 0.0f, 0.0f, 0.0f,
//		0.0f, -0.5f, 0.0f, 0.0f,
//		0.0f, 0.0f, 1.0f, 0.0f,
//		0.5f, 0.5f, 0.0f, 1.0f);
//
//	float radius = sceneBounds.radius;
//	Vector3 center = sceneBounds.center;
//
//	//for (int i = 0; i < CascadesCount; --i)
//	for (int i = CascadesCount - 1; i >= 0; --i)
//	{
//		Vector3 lightDir = Vector3(lightBufData->dirLight.direction);
//		//lightDir.Normalize(lightDir);
//		Vector3 lightPos = radius * lightDir + center;
//		Vector3 targetPos = center;
//
//		Matrix V = Matrix::CreateLookAt(lightPos, targetPos, Vector3::Up);
//
//		Vector3 sceneCenterLS = Vector3::Transform(targetPos, V);
//
//		float l = sceneCenterLS.x - radius * 2.0f;
//		float b = sceneCenterLS.y - radius * 2.0f;
//		float n = sceneCenterLS.z - radius * 2.0f;
//		float r = sceneCenterLS.x + radius * 2.0f;
//		float t = sceneCenterLS.y + radius * 2.0f;
//		float f = sceneCenterLS.z + radius * 2.0f;
//
//		Matrix P = DirectX::XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);
//
//		Matrix S = V * P * T;
//
//		radius /= 2.5f;
//
//		(V * P).Transpose(shadowMapProperties->transformVP[i]); //TODO:  CascadesCount - 1
//
//		S.Transpose(shadowMapProperties->transformS[i]);
//
//		float* distance = &shadowMapProperties->distances->x;
//		*(distance + i) = f * 2.0f * cos(lightDir.y);
//	}
//}