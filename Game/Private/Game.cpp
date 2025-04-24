#include "Game.h"

Game* Game::instance = nullptr;

Game::Game(HINSTANCE hInst, LPCWSTR appName) : hInstance(hInst), applicationName(appName) {

	clientWidth = 800;
	clientHeight = 800;

	display = new DisplayWin32(clientWidth, clientHeight);
	instance = this;
}

void Game::Initialize() {

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
	swapDescriptor.BufferCount = 3;
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

	int ShadowMapSize = 8192;

	//SHADOWS
	//+1 is for directional light
	for (int i = 0; i < /*lightBufData->lightsNum +*/ 1; ++i)
	{
		shadowMaps.push_back(new ShadowMap(device, ShadowMapSize, ShadowMapSize));
		shadowMaps[i]->ShadersInitialize(hWindow, device, context);
	}
	//END
}

void Game::PrepareResources() {
	HRESULT res = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	res = device->CreateRenderTargetView(backBuffer, nullptr, &renderView); // second parameter may be not nullptr if it's not for backbuffer

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

		RestoreTargets(1, &renderView, depthStencilView);

		Draw();

		swapChain->Present(1, /*DXGI_PRESENT_DO_NOT_WAIT*/ 0);

		RestoreTargets();
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
	context->ClearRenderTargetView(renderView, color);
	shaders->Draw(context);

	/*for (auto object : objects)
		object->Update(camManager);*/

	D3D11_MAPPED_SUBRESOURCE res = {};
	context->PSSetConstantBuffers(2, 1, &cascadeShadowPropsBuffer);
	context->Map(cascadeShadowPropsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);


	auto dataPtr = reinterpret_cast<float*>(res.pData);
	memcpy(dataPtr, shadowMapProperties, sizeof(shadowMapProps));

	context->Unmap(cascadeShadowPropsBuffer, 0);

	for (auto object : objects) {
		object->Draw(context, shadowMaps[0]);
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

/*
void Game::BuildShadowTransform()
{
	Vector3 center = Vector3::Zero;
	std::vector<Vector4> corners = GetFrustrumCornersWorldSpace(camManager->viewMatrix, camManager->projectionMatrix);

	for (const auto& v : corners)
	{
		center += Vector3(v);
	}
	center /= 8;

	Vector3 lightDir = Vector3(lightBufData->dirLight.direction);
	float radius = sceneBounds.radius;

	Matrix lightView = Matrix::CreateLookAt(center, center + lightDir * radius * 10, Vector3::Up);

	for (int i = CascadesCount - 1; i >= 0; --i)
	{
		Matrix T(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f);

		float minX = 1000000000;// std::numeric_limits<float>::max();
		float maxX = -1000000000;//std::numeric_limits<float>::lowest();
		float minY = 1000000000;// std::numeric_limits<float>::max();
		float maxY = -1000000000;//std::numeric_limits<float>::lowest();
		float minZ = 1000000000;// std::numeric_limits<float>::max();
		float maxZ = -1000000000;//std::numeric_limits<float>::lowest();

		for (const auto& corner : corners)
		{
			const Vector4 rtf = Vector4::Transform(corner, lightView);
			minX = minX < rtf.x ? minX : rtf.x;
			maxX = maxX > rtf.x ? maxX : rtf.x;
			minY = minY < rtf.y ? minY : rtf.y;
			maxY = maxY > rtf.y ? maxY : rtf.y;
			minZ = minZ < rtf.z ? minZ : rtf.z;
			maxZ = maxZ > rtf.z ? maxZ : rtf.z;
		}

		constexpr float zMult = 10.0f;
		minZ = minZ < 0 ? minZ * zMult : minZ / zMult;
		maxZ = maxZ < 0 ? maxZ / zMult : maxZ * zMult;

		Matrix lightProjection = DirectX::XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, minZ, maxZ);

		for (auto object : objects)
		{
			(lightView * lightProjection).Transpose(object->shadowMapProperties->transformVP[i]);


			(lightView * lightProjection * T).Transpose(object->shadowMapProperties->transformS[i]);
		}
	}


	////lightDir.Normalize(lightDir);
	//Vector3 lightPos = sceneBounds.radius * lightDir;
	//Vector3 targetPos = sceneBounds.center;

	//Matrix V = Matrix::CreateLookAt(lightPos, targetPos, Vector3::Up);

	//Vector3 sceneCenterLS = Vector3::Transform(targetPos, V);

	//float l = sceneCenterLS.x - sceneBounds.radius * 2.0f;
	//float b = sceneCenterLS.y - sceneBounds.radius * 2.0f;
	//float n = sceneCenterLS.z - sceneBounds.radius * 2.0f;
	//float r = sceneCenterLS.x + sceneBounds.radius * 2.0f;
	//float t = sceneCenterLS.y + sceneBounds.radius * 2.0f;
	//float f = sceneCenterLS.z + sceneBounds.radius * 2.0f;

	//Matrix P = DirectX::XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	//Matrix T(
	//	0.5f, 0.0f, 0.0f, 0.0f,
	//	0.0f, -0.5f, 0.0f, 0.0f,
	//	0.0f, 0.0f, 1.0f, 0.0f,
	//	0.5f, 0.5f, 0.0f, 1.0f);

	//for (auto object : objects)
	//{

	//	Matrix S = V * P * T;

	//	(V * P).Transpose(object->shadowMapProperties->transformVP[0]); //TODO:  CascadesCount - 1

	//	S.Transpose(object->properties->transformS);

	//	for (int i = CascadesCount - 2; i >= 0; --i)
	//	{
	//		std::vector<Vector4> corners = GetFrustrumCornersWorldSpace(camManager->viewMatrix, camManager->projectionMatrix);

	//		Vector3 center = Vector3::Zero;
	//		for (auto corner : corners)
	//		{
	//			center += Vector3(corner);
	//		}
	//		center /= corners.size();

	//		const Matrix lightView = Matrix::CreateLookAt(center, center + lightDir, Vector3::Up);

	//		Matrix VCascade = lightView;

	//		Matrix PCascade = lightProjection;

	//		(VCascade * PCascade).Transpose(object->shadowMapProperties->transformVP[i]);
	//	}
	//}
}
*/

/*
void Game::BuildShadowTransform()
{

	Matrix T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	float radius = sceneBounds.radius;
	Vector3 center = sceneBounds.center;

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

	radius /= 2.0f;

	for (auto object : objects)
	{
		(V * P).Transpose(object->shadowMapProperties->transformVP[0]); //TODO:  CascadesCount - 1

		S.Transpose(object->properties->transformS[0]);

		object->shadowMapProperties->distances[0] = radius;
	}
}
*/


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

		radius /= 2.0f;

		V.Transpose(shadowMapProperties->transformV[i]);
		(V * P).Transpose(shadowMapProperties->transformVP[i]); //TODO:  CascadesCount - 1

		S.Transpose(shadowMapProperties->transformS[i]);

		float* distance = &shadowMapProperties->distances->x;
		*(distance + i) = f * 2.0f * cos(lightDir.y);

		//std::vector<Vector4> corners = GetFrustrumCornersWorldSpace(camManager->viewMatrix, camManager->projectionMatrix);

		//Vector3 center = Vector3::Zero;
		//for (auto corner : corners)
		//{
		//	center += Vector3(corner);
		//}
		//center /= corners.size();

		//const Matrix lightView = Matrix::CreateLookAt(center, center + lightDir, Vector3::Up);

		//float minX = 1000000000;// std::numeric_limits<float>::max();
		//float maxX = -1000000000;//std::numeric_limits<float>::lowest();
		//float minY = 1000000000;// std::numeric_limits<float>::max();
		//float maxY = -1000000000;//std::numeric_limits<float>::lowest();
		//float minZ = 1000000000;// std::numeric_limits<float>::max();
		//float maxZ = -1000000000;//std::numeric_limits<float>::lowest();

		//for (const auto& corner : corners)
		//{
		//	const Vector4 rtf = Vector4::Transform(corner, lightView);
		//	minX = minX < rtf.x ? minX : rtf.x;
		//	maxX = maxX > rtf.x ? maxX : rtf.x;
		//	minY = minY < rtf.y ? minY : rtf.y;
		//	maxY = maxY > rtf.y ? maxY : rtf.y;
		//	minZ = minZ < rtf.z ? minZ : rtf.z;
		//	maxZ = maxZ > rtf.z ? maxZ : rtf.z;
		//}

		//constexpr float zMult = 10.0f;
		//minZ = minZ < 0 ? minZ * zMult : minZ / zMult;
		//maxZ = maxZ < 0 ? maxZ / zMult : maxZ * zMult;

		//Matrix lightProjection = DirectX::XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, minZ, maxZ);

		//Matrix VCascade = lightView;

		//Matrix PCascade = lightProjection;

		//(VCascade * PCascade).Transpose(object->shadowMapProperties->transformVP[i]);
	}
}


//NEW
std::vector<Vector4> Game::GetFrustrumCornersWorldSpace(const Matrix& V, const Matrix& P)
{
	Matrix VP = V * P;
	VP.Invert(VP);

	std::vector<Vector4> fCorners;
	fCorners.reserve(8);
	for (int x = 0; x < 2; ++x) {
		for (int y = 0; y < 2; ++y) {
			for (int z = 0; z < 2; ++z) {
				const Vector4 pt = Vector4::Transform(Vector4(2.0f * x - 1.0f, 2.0f * y - 1.0f, (float) z, 1.0f), VP);
				fCorners.push_back(pt / pt.w);
			}
		}
	}

	return fCorners;
}