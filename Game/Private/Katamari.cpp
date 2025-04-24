#include "Katamari.h"

//#define LIGHTTRACKING

Katamari::Katamari(HINSTANCE hinst, LPCTSTR hwindow) : Game(hinst, hwindow) {};

void Katamari::Initialize(UINT objCnt, UINT LightsCnt) {
	Game::Initialize();

	std::vector<Vector4> colors;

	Vector4 color((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f, 1.0f);
	colors.push_back(Vector4(color.x, color.y, color.z,color.w));
	colors.push_back(Vector4(color.z, color.y, color.x,color.w));

	color = Vector4((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f, 1.0f);
	colors.push_back(Vector4(color.x, color.y, color.z, color.w));
	colors.push_back(Vector4(color.z, color.y, color.x, color.w));

	color = Vector4((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f, 1.0f);
	colors.push_back(Vector4(color.x, color.y, color.z, color.w));
	colors.push_back(Vector4(color.z, color.y, color.x, color.w));

	color = Vector4((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f, 1.0f);
	colors.push_back(Vector4(color.x, color.y, color.z, color.w));
	colors.push_back(Vector4(color.z, color.y, color.x, color.w));

	objectTypes.push_back(Cube);
	objects.push_back(new GameComponent());
	materialTypes.push_back(Metal);
	objects[objects.size() - 1]->Initialize(device, objectTypes[objects.size() - 1], materialTypes[materialTypes.size() - 1], colors, LOD);
	objects[objects.size() - 1]->collisionType = Static;
	/*for (int i = 0; i < objects[0]->pointsCnt; ++i)
		objects[0]->(*points)[i].normal = Vector3(0.0f, 1.0f, 0.0f);*/
	for (int i = 0; i < 500; ++i)
	{
		objectTypes.push_back(Sphere);
		objects.push_back(new GameComponent());
		materialTypes.push_back(Glass);
		objects[objects.size() - 1]->Initialize(device, objectTypes[objects.size() - 1], materialTypes[materialTypes.size() - 1], colors, LOD);
		objects[objects.size() - 1]->collisionType = Dynamic;
	}

	std::vector<std::string> modelPaths;
	std::vector<const wchar_t*> texturePaths;

	modelPaths.push_back("Game\\Objects\\banana.fbx");
	texturePaths.push_back((const wchar_t*)L"Game\\Objects\\banana.png");
	modelPaths.push_back("Game\\Objects\\blackberry.fbx");
	texturePaths.push_back((const wchar_t*)L"Game\\Objects\\blackberry.png");
	modelPaths.push_back("Game\\Objects\\cherry.fbx");
	texturePaths.push_back((const wchar_t*)L"Game\\Objects\\cherry.png");
	modelPaths.push_back("Game\\Objects\\coconut.fbx");
	texturePaths.push_back((const wchar_t*)L"Game\\Objects\\coconut.png");
	modelPaths.push_back("Game\\Objects\\coconut_green.fbx");
	texturePaths.push_back((const wchar_t*)L"Game\\Objects\\coconut_green.png");
	modelPaths.push_back("Game\\Objects\\grape.fbx");
	texturePaths.push_back((const wchar_t*)L"Game\\Objects\\grape.png");
	modelPaths.push_back("Game\\Objects\\lemon.fbx");
	texturePaths.push_back((const wchar_t*)L"Game\\Objects\\lemon.png");
	modelPaths.push_back("Game\\Objects\\mango.fbx");
	texturePaths.push_back((const wchar_t*)L"Game\\Objects\\mango.png");
	modelPaths.push_back("Game\\Objects\\orange.fbx");
	texturePaths.push_back((const wchar_t*)L"Game\\Objects\\orange.png");
	modelPaths.push_back("Game\\Objects\\pear.fbx");
	texturePaths.push_back((const wchar_t*)L"Game\\Objects\\pear.png");
	modelPaths.push_back("Game\\Objects\\pineapple.fbx");
	texturePaths.push_back((const wchar_t*)L"Game\\Objects\\pineapple.png");
	modelPaths.push_back("Game\\Objects\\strawberry.fbx");
	texturePaths.push_back((const wchar_t*)L"Game\\Objects\\strawberry.png");

	/*modelPaths.push_back("Game\\Fruits\\apple.obj");
	texturePaths.push_back((const wchar_t*)L"Game\\Fruits\\apple.png");
	modelPaths.push_back("Game\\Fruits\\lemon.obj");
	texturePaths.push_back((const wchar_t*)L"Game\\Fruits\\lemon.png");
	modelPaths.push_back("Game\\Fruits\\pear.obj");
	texturePaths.push_back((const wchar_t*)L"Game\\Fruits\\pear.png");
	modelPaths.push_back("Game\\Fruits\\grapefruit.obj");
	texturePaths.push_back((const wchar_t*)L"Game\\Fruits\\grapefruit.png");
	modelPaths.push_back("Game\\Fruits\\pineapple.obj");
	texturePaths.push_back((const wchar_t*)L"Game\\Fruits\\pineapple.png");*/
	UINT firstModelIndex = objects.size() - 1;
	UINT modelsCnt = 0;
	if ((int) objCnt - (int) objects.size() > 0)
		modelsCnt = (int)modelPaths.size() < (int)objCnt - (int)objects.size() ? (UINT) modelPaths.size() : (UINT)(objCnt - objects.size());

	for (UINT i = 0; i < modelsCnt; ++i) {
		objectTypes.push_back(Mesh);
		materialTypes.push_back(Plastic);
		objects.push_back(new GameComponent());
		objects[objects.size() - 1]->Initialize(device, materialTypes[materialTypes.size() - 1], modelPaths[i], texturePaths[i]);
		objects[objects.size() - 1]->collisionType = Dynamic;
	}

	for (UINT i = (int) objects.size(); i < objCnt; ++i) {
		objectTypes.push_back(Mesh);
		objects.push_back(new GameComponent(device, *objects[rand() % modelPaths.size() + firstModelIndex]));
		objects[objects.size() - 1]->collisionType = Dynamic;
	}

	lightBufData = new LightningData();
	lightBufData->lightsNum = LightsCnt;

	shadowMapProperties = new shadowMapProps();

	mapSize = 40000;

	ResetGame();

	for (int i = 0; i < 100; ++i)
		transpObjects.push_back(std::vector<GameComponent*>(mapSize));
}

void Katamari::Update(float deltaTime)
{
	Game::Update(deltaTime);

	camManager->UpdatePos(input, objects[camManager->objectToTrack]->translation, clientWidth, clientHeight, &objects[camManager->objectToTrack]->impulse);
	controller->UpdatePos(input, objects[camManager->objectToTrack]);

#ifdef LIGHTTRACKING
	for (int i = 0; i < lightBufData->lightsNum; ++i)
	{
		lightBufData->pointLights[i].position = Vector4::Transform(lightsPos[i], objects[1]->rotation) + objects[1]->translation;
	}
#endif

	lightBufData->eyePos = camManager->cameraPos;
	shadowMapProperties->playerPos = Vector4(objects[1]->translation);

	//printf("%f %f %f\n", camManager->cameraPos.x, camManager->cameraPos.y, camManager->cameraPos.z);

	for (int i = 0; i < objects.size(); ++i) {
		if (objects[i]->collisionType != Dynamic)
			continue;

		if (objects[i]->collisionEnabled && objects[camManager->objectToTrack]->sphereCollider.Intersects(objects[i]->sphereCollider) && objects[camManager->objectToTrack]->sphereCollider.Radius > objects[i]->sphereCollider.Radius) {
			++objectsInside; 
			objects[i]->parent = objects[1];
			objects[i]->collisionEnabled = false;
			objects[i]->period = Vector3::Distance(objects[i]->translation - objects[camManager->objectToTrack]->translation, Vector3::Zero); //?
			objects[i]->distanceToParent = Vector3::Transform(objects[i]->translation - objects[camManager->objectToTrack]->translation, objects[camManager->objectToTrack]->rotation);


			objects[camManager->objectToTrack]->sphereCollider.Radius += objects[i]->sphereCollider.Radius / (objectsInside * 2.71828f);
			objects[camManager->objectToTrack]->translation.y += objects[i]->sphereCollider.Radius / (objectsInside * 2.71828f);
			camManager->distance = objects[camManager->objectToTrack]->sphereCollider.Radius * 10;

			Quaternion Inv;
		}
		else if (!objects[i]->collisionEnabled && objects[i]->parent != nullptr) {
			objects[i]->translation = Vector3::Transform(objects[i]->distanceToParent, objects[camManager->objectToTrack]->rotation);
		}
	}

	for (auto object : objects)
		object->Update(camManager);

	sceneBounds.center = objects[camManager->objectToTrack]->translation;

	sceneBounds.radius = 20000.0f;
}

void Katamari::DrawTransparent()
{
	int distance;
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
	for (int i = mapSize; i >= 0; ++i)
	{
		for (int j = 0; j < 100; ++j)
		{
			if (transpObjects[j][i] == NULL) 
			{
				break;
			}
			else
				transpObjects[j][i]->Draw(context, shadowMaps[0]);
		}
	}
}

void Katamari::ResetGame()
{
	objects[0]->scale = Vector3(20000.0f, 0.1f, 20000.0f);

	objects[1]->scale = Vector3(100.0f, 100.0f, 100.0f);
	objects[1]->translation = Vector3(0.0f, 100.0f, 0.0f);
	objects[1]->collisionEnabled = true;
	objects[1]->isMovable = true;
	objects[1]->isPlayerControllable = true;
	objects[1]->speed = 1.0f;
	objects[1]->initialSpeed = 1.0f;
	objects[1]->speedMax = 100.0f;

	objects[1]->sphereCollider.Radius = objects[1]->scale.x;
	objects[1]->sphereCollider.Center = objects[1]->translation;

	std::random_device dev;
	std::mt19937 rng(dev());
	int randomRange = (int) mapSize;
	std::uniform_int_distribution<std::mt19937::result_type> random1(0, randomRange);


	for (int i = 2; i < objects.size(); ++i) {
		float offset = rand() % 300;
		float scale = rand() % 1000 + 100.0f;
		objects[i]->scale = Vector3(scale, scale, scale);
		objects[i]->translation = Vector3((float)random1(rng) - randomRange / 2, objects[i]->dimensions.y * scale, (float)random1(rng) - randomRange / 2);
		objects[i]->sphereCollider.Radius = Vector3::Distance(objects[i]->dimensions, Vector3::Zero) * objects[i]->scale.x / 3;
		objects[i]->sphereCollider.Center = objects[i]->translation;
		objects[i]->isMovable = true;
		objects[i]->collisionEnabled = true;
	}

	float aspectRatio = (float) clientWidth / (float) clientHeight;
	camManager->Initialize(aspectRatio);
	camManager->objectToTrack = 1;
	camManager->isStationary = false;

	camManager->SetCameraView(objects[camManager->objectToTrack]->translation, Vector3(0.0f, objects[camManager->objectToTrack]->scale.x * 2.0f, objects[camManager->objectToTrack]->scale.x * 7.0f));

	lightBufData->dirLight.ambient = Vector4(0.1f, 0.1f, 0.1f, 1.0f);
	lightBufData->dirLight.diffuse = Vector4(0.25f, 0.25f, 0.25f, 1.0f);
	lightBufData->dirLight.specular = Vector4(0.25f, 0.25f, 0.25f, 1.0f);
	lightBufData->dirLight.direction = Vector4(0.5f, -0.5f, 0.5f, 1.0f);
	
	for (int i = 0; i < lightBufData->lightsNum; ++i)
	{
		lightBufData->pointLights[i].ambient = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		lightBufData->pointLights[i].diffuse = Vector4(0.007f, 0.007f, 0.007f, 1.0f);
		lightBufData->pointLights[i].specular = Vector4(0.07f, 0.07f, 0.07f, 1.0f);
		lightBufData->pointLights[i].attenuation = Vector4(0.0f, 0.0003f, 0.0f, 5000.0f);
		lightBufData->pointLights[i].position = Vector4((float)random1(rng) - randomRange / 2, 1000.0f, (float)random1(rng) - randomRange / 2, 1.0f);
		lightsPos.push_back(Vector3(rand() % 3000 - 1500.0f, rand() % 3000 - 1500.0f, rand() % 3000 - 1500.0f));

#ifdef LIGHTTRACKING
		lightsPos[i].Normalize();
		lightsPos[i] *= objects[camManager->objectToTrack]->sphereCollider.Radius;
#endif
		//
		//lightBufData->pointLights[i].specular = Vector4((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f, 1.0f);
		// - 50 is for pretty colors, but thats wrong XD
		lightBufData->pointLights[i].specular = Vector4(((rand() % 100) - 50) / 100.0f, ((rand() % 100) - 50) / 100.0f, ((rand() % 100) - 50) / 100.0f, 1.0f);
		//lightPosL = Vector4(objects[2]->translation);
	}

	//NEW
}

void Katamari::Draw()
{
	Game::Draw();

	DrawTransparent();
}
