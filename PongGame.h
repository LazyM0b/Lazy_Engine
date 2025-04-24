#pragma once

#include "Game.h"
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

class PongGame : public Game {
public:
	PongGame(HINSTANCE hinst, LPCTSTR hwindow);
	void Initialize(UINT objCnt);
	void Update(float deltaTime) override;
	void ShowScores();
	void ResetGame();

	UINT score1 = 0;
	UINT score2 = 0;
};