#include "SolarSystem.h"

SolarSystem::SolarSystem(HINSTANCE hinst, LPCTSTR hwindow) : Game(hinst, hwindow) {};

void SolarSystem::Initialize(UINT objCnt) {
	Game::Initialize(objCnt);

	for (UINT i = 0; i < objCnt; ++i) {

		std::vector<Vector4> colors;

		Vector3 color((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f);
		colors.push_back(Vector4(color.x, color.y, color.z, 1.0f));
		colors.push_back(Vector4(color.z, color.y, color.x, 1.0f));

		objects.push_back(new GameComponent()); 
		if (i < objCnt - 200) {
			objectTypes.push_back(Sphere);
			materialTypes.push_back(Metal);
			objects[i]->Initialize(device, objectTypes[i], materialTypes[i], colors, LOD);
		}
		else {
			objectTypes.push_back(Strip);
			materialTypes.push_back(Metal);
			objects[i]->Initialize(device, objectTypes[i], materialTypes[i], colors, LOD);
		}
	}
	for (UINT i = 0; i < objCnt - 200; ++i)
		objects[i]->isMovable = true;

	ResetGame();
}

void SolarSystem::Update(float deltaTime)
{
	Game::Update(deltaTime);

	for (int i = 0; i < objects.size(); ++i) {

		//planets rotation
		objects[i]->rotation = Quaternion::CreateFromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), objects[i]->rotation.ToEuler().y + objects[i]->speed);

		//world position calculation
		if (objects[i]->parent != nullptr) 
			//orbital movement
			objects[i]->translation = Vector3(cos(objects[i]->rotation.ToEuler().y + objects[i]->period), 0.0f, sin(objects[i]->rotation.ToEuler().y + objects[i]->period)) * objects[i]->distanceToParent;

		if (i == camManager->objectToTrack) {

			//camera rotation
			//camManager->SetCameraView(objects[camManager->objectToTrack]->translation, Vector3(objects[camManager->objectToTrack]->scale.x * 7, objects[camManager->objectToTrack]->scale.x * 2, 0.0f));
		}
	}

	if (!SwitchPlanet()) {
		if (camManager->objectToTrack != -1) {
			camManager->RotateCamera(Quaternion::CreateFromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), 0.00714f));
			camManager->UpdatePos(input, objects[camManager->objectToTrack]->translation, clientWidth, clientHeight);
		}
		else
			camManager->UpdatePos(input, camManager->objectPos, clientWidth, clientHeight);
	}

}

void SolarSystem::ResetGame()
{
	//Initialize planets scales
	for (int i = 0; i < objects.size(); ++i) {
		if (i < objects.size() - 200) {
			objects[i]->scale = Vector3((i + 5.0f) * 100.0f, (i + 5.0f) * 100.0f, (i + 5.0f) * 100.0f);
			objects[i]->rotation = Quaternion::CreateFromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), 0.0f);
			objects[i]->translation = Vector3(i * 7500.0f, 2000.0f, 0.0f);
			objects[i]->period = rand() % 600 / 100.0f;
			objects[i]->speed = rand() % 100 / 10000.0f;

			if (i != 0) {
				objects[i]->parent = objects[0];
				objects[i]->distanceToParent = Vector3(objects[i]->scale.x * i * 3, 0.0f, objects[i]->scale.x * i * 3);
			}

			if (i == 4 || i == 8 || i == 9) {
				objects[i]->parent = objects[i - 1];
				objects[i]->speed *= rand() % 5 + 2;
				objects[i]->distanceToParent = Vector3(objects[i]->scale.x * (rand() % 3 + 3.0f), 0.0f, objects[i]->scale.x * (rand() % 3 + 3.0f));
			}
		}
		else {
			if (i < objects.size() - 100) {
				objects[i]->scale = Vector3(100000.0f, 0.0f, 0.0f);
				objects[i]->translation = Vector3(0.0f, 0.0f, (i - (int)objects.size() + 150.0f) * 1000.0f);
			}
			else {
				objects[i]->scale = Vector3(100000.0f, 0.0f, 0.0f);
				objects[i]->translation = Vector3((i - (int)objects.size() + 50.0f) * 1000.0f, 0.0f, 0.0f);
				objects[i]->rotation = Quaternion::CreateFromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), asin(1.0f));
			}
		}
	}

	float aspectRatio = (float)clientWidth / (float)clientHeight;
	camManager->Initialize(aspectRatio);

	camManager->objectToTrack = 0;
	camManager->SetCameraView(objects[camManager->objectToTrack]->translation, Vector3(objects[camManager->objectToTrack]->scale.x * 7, objects[camManager->objectToTrack]->scale.x * 2, 0.0f));

	lightBufData = new LightningData();

	lightBufData->pointLights[0].diffuse = Vector4(0.1f, 0.7f, 0.1f, 1.0f);
	lightBufData->pointLights[0].specular = Vector4(0.1f, 0.7f, 0.1f, 1.0f);
	lightBufData->pointLights[0].attenuation = Vector4(0.0f, 0.0001f, 0.0f, 500000.0f);
	lightBufData->Data.x = 1;

	//lightPosL = Vector4(objects[0]->translation);
}

bool SolarSystem::SwitchPlanet() {
	int keyNum = 0;
	if (input->IsKeyDown(Keys::D0)) {
		keyNum = int(Keys::D0);
		input->RemovePressedKey(Keys::D0);
		}
	else if (input->IsKeyDown(Keys::D1)) {
		keyNum = int(Keys::D1);
		input->RemovePressedKey(Keys::D1);
	}
	else if (input->IsKeyDown(Keys::D2)) {
		keyNum = int(Keys::D2);
		input->RemovePressedKey(Keys::D2);
	}
	else if (input->IsKeyDown(Keys::D3)) {
		keyNum = int(Keys::D3);
		input->RemovePressedKey(Keys::D3);
	}
	else if (input->IsKeyDown(Keys::D4)) {
		keyNum = int(Keys::D4);
		input->RemovePressedKey(Keys::D4);
	}
	else if (input->IsKeyDown(Keys::D5)) {
		keyNum = int(Keys::D5);
		input->RemovePressedKey(Keys::D5);
	}
	else if (input->IsKeyDown(Keys::D6)) {
		keyNum = int(Keys::D6);
		input->RemovePressedKey(Keys::D6);
	}
	else if (input->IsKeyDown(Keys::D7)) {
		keyNum = int(Keys::D7);
		input->RemovePressedKey(Keys::D7);
	}
	else if (input->IsKeyDown(Keys::D8)) {
		keyNum = int(Keys::D8);
		input->RemovePressedKey(Keys::D8);
	}
	else if (input->IsKeyDown(Keys::D9)) {
		keyNum = int(Keys::D9);
		input->RemovePressedKey(Keys::D9);
	}
	else if (input->IsKeyDown(Keys::Space)) {
		keyNum = -1;
		input->RemovePressedKey(Keys::Space);
	}

	if (keyNum == -1) {
		camManager->objectToTrack = -1;
		camManager->isStationary = 0;
		camManager->SetCameraView(Vector3::Zero, Vector3(0.0f, 5000.0f, 0.0f));
		return true;
	}
	else if (keyNum > 0 && keyNum - 48 != camManager->objectToTrack) {
		camManager->objectToTrack = keyNum - 48;
		camManager->isStationary = 1;
		camManager->SetCameraView(objects[camManager->objectToTrack]->translation, Vector3(objects[camManager->objectToTrack]->scale.x * 7, objects[camManager->objectToTrack]->scale.x * 2, 0.0f));
		return true;
	}
	else 
		return false;
}
