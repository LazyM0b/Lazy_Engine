#pragma once

#include "Game.h"
#include "SimpleMath.h"
#include "random"

using namespace DirectX::SimpleMath;

class Katamari : public Game {
public:
	Katamari(HINSTANCE hinst, LPCTSTR hwindow);
	virtual void Initialize(UINT objCnt, UINT pointlightsCnt = 0, UINT spotlightsCnt = 0, UINT mapSize = 0) override;
	void Update(float deltaTime) override;
	void ResetGame();

	UINT objectsInside = 1;
	UINT planetsCnt = 10;
};