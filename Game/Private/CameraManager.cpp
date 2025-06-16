#include "CameraManager.h"

CameraManager::CameraManager() {}

CameraManager::CameraManager(float dist)
{
	distance = dist;
	cameraPos = Vector3(dist, dist, 0.0f);
	cameraOffset = Vector3::Forward;
	cameraRotation = Quaternion::CreateFromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), 0.0f);
}

void CameraManager::SetCameraView(Vector3 objPos, Vector3 camPos)
{
	objectPos = objPos;
	Mat =Matrix::CreateFromYawPitchRoll(Vector3(acos(camPos.x / distance), asin(camPos.y / distance), asin(camPos.z / distance)));
	cameraPos = Vector3::Transform(camPos, cameraRotation) + objectPos;
	distance = Vector3::Distance(cameraPos, objectPos);

	if (objectPos == cameraPos)
		cameraPos.z -= 1.0f;
	viewMatrix = Matrix::CreateLookAt(cameraPos, objectPos, Vector3::Up);
	cameraRotation = Quaternion::CreateFromYawPitchRoll(camPos / distance);
}

void CameraManager::RotateCamera(Quaternion rotVector)
{
	cameraRotation = rotVector;
}

void CameraManager::UpdatePos(InputDevice* input, const Vector3& objPos,UINT width, UINT height, Vector3* playerImpulse)
{
	Vector3 camPos;
	//updating free camera fly speed
	objectPos = objPos;

	forwardVector = cameraPos - objectPos;
	forwardVector.Normalize();

	if (!isStationary) {
		if (cameraOffset == Vector3::Zero)
			cameraOffset = Vector3::Right;

		if (input->MouseOffset.x) {
			cursorPos.x += input->MouseOffset.x * mouseMoveSpeed;
			cameraOffset.x = cos((cursorPos.x * 6.28f) / (width / 2));
			cameraOffset.z = sin((cursorPos.x * 6.28f) / (width / 2));

			//define player impulse vector
			if (playerImpulse != nullptr)
				*playerImpulse = Vector3(cos((cursorPos.x * 6.28f) / (width / 2)), 0.0f, sin((cursorPos.x * 6.28f) / (width / 2)));
		}
		if (input->MouseOffset.y) {
			cursorPos.y -= input->MouseOffset.y * mouseMoveSpeed * 4;
			if (cursorPos.y > float(height))
				cursorPos.y = float(height);
			else if (cursorPos.y < float(height) * -1)
				cursorPos.y = float(height) * -1;
			cameraOffset.y = sin(-1 * (cursorPos.y * 1.57f) / height);
		}
		input->MouseOffset = Vector2::Zero;

		if (objectToTrack == -1) {
			distance = 1.0f;

			if (input->MouseWheelDelta != 0) {
				camMoveSpeed = camMoveSpeed + input->MouseWheelDelta * 0.1f;
				if (camMoveSpeed < 0.0f)
					camMoveSpeed = 0.0f;
			}

			float moveSpeed = camMoveSpeed;

			if (input->IsKeyDown(Keys::LeftShift))
				moveSpeed *= 2;

			if (input->IsKeyDown(Keys::W)) {
				objectPos -= forwardVector * moveSpeed;
			}
			if (input->IsKeyDown(Keys::S)) {
				objectPos += forwardVector * moveSpeed;
			}
			if (input->IsKeyDown(Keys::A)) {
				objectPos.x += cos(asin(1.0f) + forwardVector.z) * moveSpeed;
				objectPos.z += sin(acos(1.0f) + forwardVector.x) * moveSpeed;
			}
			if (input->IsKeyDown(Keys::D)) {
				objectPos.x += cos(asin(-1.0f) + forwardVector.z) * moveSpeed;
				objectPos.z += sin(acos(-1.0f) + forwardVector.x) * moveSpeed;
			}
			if (input->IsKeyDown(Keys::Q)) {
				objectPos.y += moveSpeed;
			}
			if (input->IsKeyDown(Keys::E)) {
				objectPos.y += moveSpeed * -1;
			}
		}
		camPos = cameraOffset;
		camPos.Normalize();
	}
	else
		camPos = forwardVector;

	SetCameraView(objectPos, camPos * distance);
}

void CameraManager::Initialize(float aspectRatio, int FOV /*= 1.57f*/)
{
	float nearPlane = 0.5f;
	float farPlane = 20000.0f;
	projectionMatrix = Matrix::CreatePerspectiveFieldOfView(FOV, aspectRatio, nearPlane, farPlane);
}