#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <chrono>
#include <vector>
#include <limits>
#include <iostream>

#include "DisplayWin32.h"
#include "ShadersComponent.h"
#include "InputDevice.h"
#include "CameraManager.h"
#include "PlayerController.h"
#include "DeferredRenderingComponent.h"
#include "ShadowsComponent.h"
#include "GameComponent.h"
#include "SimpleMath.h"
#include "Math.h"
#include "conio.h"
//#include "GameTimer.h"

using namespace DirectX::SimpleMath;

class Game {
public:

	Game(HINSTANCE hinst, LPCWSTR appName);
	virtual void Initialize(UINT objCnt, UINT lightsCnt = 0, UINT mapSize = 0);
	void PrepareResources();
	int MessageHandler(UINT msg);
	void Run();
	virtual void Draw();
	void PrepareViewport();
	void PrepareOpaque();
	void PrepareLighting();
	void PrepareTransparent();
	virtual void Update(float deltaTime);
	void UpdateInternal(); //?
	void RestoreTargets(int viewsCnt = 0, ID3D11RenderTargetView* const* RenderView = nullptr, ID3D11DepthStencilView* DepthStencilView = nullptr); // done
	void EndFrame(); //?
	const HWND& MainWindow();
	int Exit();
	void DestroyResources();
	//SHADOWS
	void DrawSceneToShadowMap();
	void BuildShadowTransform();
	//END

	DisplayWin32* display;
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	HINSTANCE hInstance; 
	LPCWSTR applicationName;
	HWND hWindow;
	static Game* instance;
	DeferredSystem* renderingSystem;

	std::vector<GameComponent*> objects;
	std::vector<MeshTypes> objectTypes;
	std::vector<MaterialTypes> materialTypes;
	ShadersComponent* shaders;
	InputDevice* input;

	DXGI_SWAP_CHAIN_DESC swapDescriptor;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swapChain;
	ID3D11Texture2D* backBuffer;
	ID3D11Texture2D* depthStencilBuffer;
	ID3D11RenderTargetView* renderView;
	ID3D11DepthStencilView* depthStencilView; 
	ID3D11BlendState* blendState;

	//LIGHT
	ID3D11Buffer* lightBuf;
	LightningData* lightBufData;
	std::vector<Vector3> lightsPos;
	//
	
	//SHADOWS
	std::vector<ShadowMap*> shadowMaps;
	BoundingSphere sceneBounds;
	shadowMapProps* shadowMapProperties;
	ID3D11Buffer* cascadeShadowPropsBuffer;
	//END

	CameraManager* camManager;
	PlayerController* controller;
	//GameTimer timer;

	std::chrono::time_point<std::chrono::steady_clock> PrevTime;
	float totalTime = 0;
	UINT LOD = 1;
	UINT frameCount = 0;
	UINT clientWidth;
	UINT clientHeight;
	UINT mapSize;
};