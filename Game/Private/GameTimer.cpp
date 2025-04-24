#include "GameTimer.h"

GameTimer::GameTimer() : /*secondsPerCount(1.0f),*/ deltaTime(1.0f), baseTime(0), pausedTime(0), prevTime(0), stopTime(0), curTime(0), stopped(false) {
	UINT countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	//secondsPerCount = 1.0 / (float) countsPerSec;
};

void GameTimer::Tick() {
	if (stopped) {
		deltaTime = 0.0f;
		return;
	}

	float curTimer;
	QueryPerformanceCounter((LARGE_INTEGER*)&curTimer);
	curTime = curTimer;

	deltaTime = (curTime - prevTime); //* secondsPerCount;

	prevTime = curTime;

	if (deltaTime < 0.0f)
		deltaTime = 0.0f;
}

float GameTimer::DeltaTime() const {
	return deltaTime;
}

void GameTimer::Start() {
	if (stopped) {
		UINT startTimer;
		QueryPerformanceCounter((LARGE_INTEGER*)&startTimer);

		pausedTime += startTimer - stopTime;

		prevTime = startTimer;
		stopTime = 0;
		stopped = false;
	}
}

void GameTimer::Stop() {
	if (!stopped) {
		UINT curTimer;
		QueryPerformanceCounter((LARGE_INTEGER*)&curTimer);

		stopTime = curTimer;
		stopped = true;
	}
}

void GameTimer::Reset() {
	UINT curTimer;
	QueryPerformanceCounter((LARGE_INTEGER*)&curTimer);

	baseTime = curTimer;
	prevTime = curTimer;
	stopTime = 0;
	stopped = false;
}
