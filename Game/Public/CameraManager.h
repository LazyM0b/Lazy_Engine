#pragma once

#include "SimpleMath.h"
#include "InputDevice.h"

using namespace DirectX::SimpleMath;

class CameraManager {
public:
	CameraManager();
	CameraManager(float dist);
	void SetCameraView(Vector3 objPos, Vector3 camPos);
	void RotateCamera(Quaternion rotVector);
	void UpdatePos(InputDevice* input, const Vector3& objPos, UINT width, UINT height, Vector3* playerImpulse = nullptr);
	void Initialize(float aspectRatio, int FOV = 1.57f);

	int objectToTrack = -1;
	bool isStationary = 1;
	float distance;
	float camMoveSpeed = 20.1f;
	float camRotationSpeed = 0.1f;
	float mouseMoveSpeed = 1.0f;
	Vector3 forwardVector;
	Vector3 objectPos;
	Vector3 cameraPos;
	Vector3 cameraOffset;
	Vector2 cursorPos;
	Quaternion cameraRotation;
	Matrix viewMatrix;
	Matrix projectionMatrix;
};