#pragma once
#include <Windows.h>

class GameTimer {
public:
	GameTimer();

	float GameTime() const;
	float DeltaTime() const;

	void Reset();
	void Start();
	void Stop();
	void Tick();

private:
	//float secondsPerCount;
	float deltaTime;

	UINT baseTime;
	UINT pausedTime;
	UINT stopTime;
	UINT prevTime;
	UINT curTime;

	bool stopped;
};