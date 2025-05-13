#include "GameComponent.h"

GameComponent::GameComponent() {}

//copy constructor
GameComponent::GameComponent(Microsoft::WRL::ComPtr<ID3D11Device> device, const GameComponent& other)
{
	this->type = other.type;
	this->pointsCnt = other.pointsCnt;
	this->points = other.points;
	this->shadowPoints = other.shadowPoints;
	this->indexes = other.indexes;
	this->collisionEnabled = other.collisionEnabled;
	this->collisionType = other.collisionType;
	this->isMovable = other.isMovable;
	this->strides = other.strides;
	this->shadowStrides = other.shadowStrides;
	this->offsets = other.offsets;
	this->shadowOffsets = other.shadowOffsets;

	this->speed = other.speed;
	this->speedMax = other.speedMax;
	this->scale = other.scale;
	this->dimensions = other.dimensions;
	this->rotation = other.rotation;
	this->translation = other.translation;
	this->velocity = other.velocity;

	this->resource = other.resource;
	this->m_texture = other.m_texture;
	this->properties = new objectProps(*other.properties);
	this->parent = other.parent;
	this->distanceToParent = other.distanceToParent;
	TriangleComponent::Initialize(device);
}

//default constructor with white color
void GameComponent::Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device, MeshTypes type) {
	this->type = type;
	float phi = (1.0f + sqrt(5.0f)) * 0.5f; // golden ratio
	float coord1 = 1.0f;
	float coord2 = 1.0f / phi;
	UINT numSubdivide;
	properties = new objectProps();

	switch (type)
	{
	case Triangle:
		pointsCnt = 3;

		points = new std::vector<Vertex>{
		{Vector4(1.0f, 1.0f, 0.5f, 1.0f),	Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
		{Vector4(-1.0f, -1.0f, 0.5f, 1.0f),Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
		{Vector4(-1.0f, 1.0f, 0.5f, 1.0f),	Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
		};
		indexes = new std::vector<int>{ 0,1,2 };
		break;
	case Square:
		pointsCnt = 4;

		points = new std::vector<Vertex>{
		{Vector4(1.0f, 1.0f, 0.5f, 1.0f),	Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
		{Vector4(-1.0f, -1.0f, 0.5f, 1.0f),Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
		{Vector4(1.0f, -1.0f, 0.5f, 1.0f),	Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
		{Vector4(-1.0f, 1.0f, 0.5f, 1.0f),	Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
		};
		indexes = new std::vector<int>{ 0,1,2, 1,0,3 };
		break;
	case Circle:
		pointsCnt = 100;

		points = new std::vector<Vertex>{ { Vector4(0.0f, 0.0f, 0.5f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f) } };
		for (UINT i = 1; i < pointsCnt; ++i) {
			(*points).push_back(Vertex());
			(*points)[i].location = Vector4(cos((i - 1) / (pointsCnt - 2.0f) * 6.28f), sin((i - 1) / (pointsCnt - 2.0f) * 6.28f), 0.5f, 1.0f);
			(*points)[i].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
			(*indexes).push_back(0);
			(*indexes).push_back((i + 1) % pointsCnt);
			(*indexes).push_back(i);
		}
	case Cube:
		pointsCnt = 8;
		points = new std::vector<Vertex>{
			{Vector4(-1.0f, 1.0f, -1.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
			{Vector4(1.0f, 1.0f, -1.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
			{Vector4(-1.0f, -1.0f, -1.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
			{Vector4(1.0f, -1.0f, -1.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
			{Vector4(-1.0f, 1.0f, 1.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
			{Vector4(1.0f, 1.0f, 1.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
			{Vector4(-1.0f, -1.0f, 1.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
			{Vector4(1.0f, -1.0f, 1.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)}
		};
		indexes = new std::vector<int>{
			0,1,2, 1,3,2,
			0,5,1, 0,4,5,
			1,5,7, 1,7,3,
			0,6,4, 0,2,6,
			4,6,5, 5,6,7,
			2,3,6, 3,7,6
		};
	break; 
	case Sphere:

		pointsCnt = 12;
		numSubdivide = 3;

		points = new std::vector<Vertex>
		{
			{Vector4(0.0f, coord2, -coord1, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
			{Vector4(coord2, coord1, 0.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
			{Vector4(-coord2, coord1, 0.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
			{Vector4(0.0f, coord2, coord1, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
			{Vector4(0.0f, -coord2, coord1, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
			{Vector4(-coord1, 0.0f, coord2, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
			{Vector4(0.0f, -coord2, -coord1, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
			{Vector4(coord1, 0.0f, -coord2, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
			{Vector4(coord1, 0.0f, coord2, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
			{Vector4(-coord1, 0.0f, -coord2, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
			{Vector4(coord2, -coord1, 0.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)},
			{Vector4(-coord2, -coord1, 0.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)}
		};		indexes = new std::vector<int>
		{
		2,1,0, 1,2,3, 5,4,3, 4,8,3, 7,6,0,
		6,9,0, 11,10,4, 10,11,6, 9,5,2, 5,9,11,
		8,7,1, 7,8,10, 2,5,3, 8,1,3, 9,2,0,
		1,7,0, 11,9,6, 7,10,6, 5,11,4, 10,8,4
		};		for (UINT j = 0; j < numSubdivide; ++j) {			SphereSubdivide(points, indexes);			pointsCnt = (UINT) (*points).size();			for (UINT i = 0; i < pointsCnt; ++i)				PointNormalize((*points)[i]);		}
		break;
	default:
		printf("Object type wasn't specified properly");
		break;
	}

	shadowPoints = new std::vector<shadowVertex>;
	for (UINT i = 0; i < pointsCnt; ++i)
	{
		shadowVertex tmp = { (*points)[i].location };
		(*shadowPoints).push_back(tmp);
	}

	boxCollider = DirectX::BoundingBox(Vector3::Zero, Vector3::One);
	sphereCollider = DirectX::BoundingSphere(Vector3::Zero, 1.0f);

	for (UINT i = 0; i < pointsCnt; ++i) {
		int normalNum = 0;
		for (UINT j = 0; j < (*indexes).size() / 3; ++j) {
			if ((*indexes)[j * 3] == i || (*indexes)[j * 3 + 1] == i || (*indexes)[j * 3 + 2] == i) {
				Vector3 normal;
				ComputeNormal(Vector3((*points)[(*indexes)[j * 3]].location), Vector3((*points)[(*indexes)[j * 3 + 1]].location), Vector3((*points)[(*indexes)[j * 3 + 2]].location), normal);
				(*points)[i].normal += normal;
				++normalNum;
			}
		}
	}

	for (UINT i = 0; i < (*points).size(); ++i)
		(*points)[i].normal.Normalize();

	TriangleComponent::Initialize(device);
}

//constructor with color
void GameComponent::Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device, MeshTypes type, const MaterialTypes& matType, std::vector<Vector4> colors, UINT detailsLVL) {
	this->type = type; 
	float phi = (1.0f + sqrt(5.0f)) * 0.5f; // golden ratio
	float coord1 = 1.0f;
	float coord2 = 1.0f / phi;
	UINT numSubdivide;
	properties = new objectProps();

	switch (type)
	{
	case Strip:
		pointsCnt = 2;

		points = new std::vector<Vertex>{
			{ Vector4(1.0f, 0.0f, 0.0f, 1.0f), colors[0] },
			{ Vector4(-1.0f, 0.0f, 0.0f, 1.0f), colors[1] }
		};
		break;
	case Triangle:
		pointsCnt = 3;

		points = new std::vector<Vertex>{
		{Vector4(1.0f, 1.0f, 0.5f, 1.0f),	colors[0]},
		{Vector4(-1.0f, -1.0f, 0.5f, 1.0f),colors[1]},
		{Vector4(-1.0f, 1.0f, 0.5f, 1.0f),	colors[2]},
		};
		indexes = new std::vector<int>{ 0,1,2 };
		break;
	case Square:
		pointsCnt = 4;

		points = new std::vector<Vertex>{
		{Vector4(1.0f, 1.0f, 0.5f, 1.0f),	colors[0]},
		{Vector4(-1.0f, -1.0f, 0.5f, 1.0f),colors[1]},
		{Vector4(1.0f, -1.0f, 0.5f, 1.0f),	colors[2]},
		{Vector4(-1.0f, 1.0f, 0.5f, 1.0f),	colors[3]},
		};
		indexes = new std::vector<int>{ 0,1,2, 1,0,3 };
		break;
	case Circle:
		pointsCnt = 100;

		points = new std::vector<Vertex>{ { Vector4(0.0f, 0.0f, 0.5f, 1.0f), colors[0] } };
		for (UINT i = 1; i < pointsCnt; ++i) {
			(*points).push_back(Vertex());
			(*points)[i].location = Vector4(cos((i - 1) / (pointsCnt - 2.0f) * 6.28f), sin((i - 1) / (pointsCnt - 2.0f) * 6.28f), 0.5f, 1.0f);
			(*points)[i].color = colors[i];
			(*indexes).push_back(0);
			(*indexes).push_back((i + 1) % pointsCnt);
			(*indexes).push_back(i);
		}
		break;
	case Cube:
		pointsCnt = 8;
		points = new std::vector<Vertex>{
			{Vector4(-1.0f, 1.0f, -1.0f, 1.0f), colors[0]},
			{Vector4(1.0f, 1.0f, -1.0f, 1.0f), colors[1]},
			{Vector4(-1.0f, -1.0f, -1.0f, 1.0f), colors[2]},
			{Vector4(1.0f, -1.0f, -1.0f, 1.0f), colors[3]},
			{Vector4(-1.0f, 1.0f, 1.0f, 1.0f), colors[4]},
			{Vector4(1.0f, 1.0f, 1.0f, 1.0f), colors[5]},
			{Vector4(-1.0f, -1.0f, 1.0f, 1.0f), colors[6]},
			{Vector4(1.0f, -1.0f, 1.0f, 1.0f), colors[7]}
		};
		indexes = new std::vector<int>{
			0,1,2, 1,3,2,
			0,5,1, 0,4,5,
			1,5,7, 1,7,3,
			0,6,4, 0,2,6,
			4,6,5, 5,6,7,
			2,3,6, 3,7,6
		};
		break;
	case Sphere:

		pointsCnt = 12;
		numSubdivide = detailsLVL;

		points = new std::vector<Vertex>
		{
			{Vector4(0.0f, coord2, -coord1, 1.0f), colors[0] },
			{Vector4(coord2, coord1, 0.0f, 1.0f), colors[1] },
			{Vector4(-coord2, coord1, 0.0f, 1.0f), colors[0] + colors[1]},
			{Vector4(0.0f, coord2, coord1, 1.0f), colors[1] + colors[0]},
			{Vector4(0.0f, -coord2, coord1, 1.0f), colors[0] },
			{Vector4(-coord1, 0.0f, coord2, 1.0f), colors[1]},
			{Vector4(0.0f, -coord2, -coord1, 1.0f), colors[0] - colors[1]},
			{Vector4(coord1, 0.0f, -coord2, 1.0f), colors[1] - colors[0]},
			{Vector4(coord1, 0.0f, coord2, 1.0f), colors[0]},
			{Vector4(-coord1, 0.0f, -coord2, 1.0f), colors[1] },
			{Vector4(coord2, -coord1, 0.0f, 1.0f), colors[0] + colors[1]},
			{Vector4(-coord2, -coord1, 0.0f, 1.0f), colors[1] - colors[0]}
		};		indexes = new std::vector<int>
		{
		2,1,0, 1,2,3, 5,4,3, 4,8,3, 7,6,0,
		6,9,0, 11,10,4, 10,11,6, 9,5,2, 5,9,11,
		8,7,1, 7,8,10, 2,5,3, 8,1,3, 9,2,0,
		1,7,0, 11,9,6, 7,10,6, 5,11,4, 10,8,4
		};		for (UINT j = 0; j < numSubdivide; ++j) {			SphereSubdivide(points, indexes);			pointsCnt = (UINT)(*points).size();			for (UINT i = 0; i < pointsCnt; ++i)				PointNormalize((*points)[i]);		}
		break;
	default:
		break;
	}

	shadowPoints = new std::vector<shadowVertex>;
	for (UINT i = 0; i < pointsCnt; ++i)
	{
		shadowVertex tmp = { (*points)[i].location };
		(*shadowPoints).push_back(tmp);
	}

	boxCollider = DirectX::BoundingBox(Vector3::Zero, Vector3::One);
	sphereCollider = DirectX::BoundingSphere(Vector3::Zero, 1.0f);

	for (UINT i = 0; i < pointsCnt; ++i) {
		for (UINT j = 0; j < (*indexes).size() / 3; ++j) {
			if ((*indexes)[j * 3] == i || (*indexes)[j * 3 + 1] == i || (*indexes)[j * 3 + 2] == i) {
				Vector3 normal;
				ComputeNormal(Vector3((*points)[(*indexes)[j * 3]].location), Vector3((*points)[(*indexes)[j * 3 + 1]].location), Vector3((*points)[(*indexes)[j * 3 + 2]].location), normal);
				(*points)[i].normal += normal;
			}
		}
	}

	for (UINT i = 0; i < (*points).size(); ++i)
		(*points)[i].normal.Normalize();

	InitializeMaterial(matType);

	TriangleComponent::Initialize(device);
}

//initialize mesh model with asset path
void GameComponent::Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device, const MaterialTypes& matType, const std::string& modelPath, const wchar_t* texturePath)
{
	this->type = Mesh;
	if (!this->LoadModel(modelPath)) {
		std::cout << "Model not imported";
		return;
	}
	pointsCnt = (UINT)(*points).size();

	for (auto point : *points)
		point.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	shadowPoints = new std::vector<shadowVertex>;
	for (UINT i = 0; i < pointsCnt; ++i)
	{
		shadowVertex tmp = { (*points)[i].location };
		(*shadowPoints).push_back(tmp);
	}
	properties = new objectProps();
	Vector3 transMin = Vector3::One;
	Vector3 transMax = Vector3::One * -1;
	for (UINT i = 0; i < pointsCnt; ++i) {
		if ((*points)[i].location.x < transMin.x)
			transMin.x = (*points)[i].location.x;
		if ((*points)[i].location.x > transMax.x)
			transMax.x = (*points)[i].location.x;

		if ((*points)[i].location.y < transMin.y)
			transMin.y = (*points)[i].location.y;
		if ((*points)[i].location.y > transMax.y)
			transMax.y = (*points)[i].location.y;

		if ((*points)[i].location.z < transMin.z)
			transMin.z = (*points)[i].location.z;
		if ((*points)[i].location.z > transMax.z)
			transMax.z = (*points)[i].location.z;
	}
	dimensions = (transMax - transMin) / 2.0f;
	float dist = 0.0f;
	for (UINT i = 0; i < pointsCnt; ++i) {
		Vector4 trans = Vector4(transMin);
		(*points)[i].location -= trans;
		trans = Vector4(dimensions);
		(*points)[i].location -= trans;
		if (Vector4::Distance((*points)[i].location, Vector4::Zero) > dist)
			dist = Vector4::Distance((*points)[i].location, Vector4::Zero);
	}

	/*for (int i = 0; i < pointsCnt; ++i) {
		(*points)[i].location.x /= dist;
		(*points)[i].location.y /= dist;
		(*points)[i].location.z /= dist;
	}*/

	if (!this->LoadTexture(device, texturePath)) {
		std::cout << "Texture not imported";
		return;
	}

	boxCollider = DirectX::BoundingBox(Vector3::Zero, Vector3::One);
	sphereCollider = DirectX::BoundingSphere(Vector3::Zero, 1.0f);

	for (UINT i = 0; i < (*points).size(); ++i) {
		UINT normalNum = 0;
		for (UINT j = 0; j < (*indexes).size() / 3; ++j) {
			if ((*indexes)[j * 3] == i || (*indexes)[j * 3 + 1] == i || (*indexes)[j * 3 + 2] == i) {
				Vector3 normal;
				ComputeNormal(Vector3((*points)[(*indexes)[j * 3]].location), Vector3((*points)[(*indexes)[j * 3 + 1]].location), Vector3((*points)[(*indexes)[j * 3 + 2]].location), normal);
				(*points)[i].normal += normal;
				++normalNum;
			}
		}
	}

	for (UINT i = 0; i < (*points).size(); ++i)
		(*points)[i].normal.Normalize();

	InitializeMaterial(matType);

	TriangleComponent::Initialize(device);
}

void GameComponent::InitializeMaterial(const MaterialTypes& matType)
{
	switch (matType) {
	case Rubber:
		break;
	case Plastic:
		properties->material.ambient = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		properties->material.diffuse = Vector4(0.1f, 0.1f, 0.1f, 1.0f);
		properties->material.specular = Vector4(0.1f, 0.1f, 0.1f, 1.0f);
		break;
	case Metal:
		properties->material.ambient = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		properties->material.diffuse = Vector4(0.2775f, 0.2775f, 0.2775f, 1.0f);
		properties->material.specular = Vector4(0.773911f, 0.773911f, 0.773911f, 100.0f);
		break;
	case Glass:
		properties->material.ambient = Vector4(1.0f, 1.0f, 1.0f, 0.2f);
		properties->material.diffuse = Vector4(0.1f, 0.1f, 0.1f, 1.0f);
		properties->material.specular = Vector4(0.773911f, 0.773911f, 0.773911f, 150.0f);
		break;
	default:
		break;
	}

	if (properties->material.ambient.w < 1.0f)
		isTransparent = true;
}

void GameComponent::Update(CameraManager* camManager)
{
	Vector3 objRotation = this->rotation.ToEuler();
	Vector3 tmpTranslation = translation;
	GameComponent* parent = this->parent;
	while (parent != nullptr) {
		objRotation += parent->rotation.ToEuler();
		tmpTranslation += parent->translation;
		parent = parent->parent;
	}

	properties->transformW = Matrix::CreateScale(scale);
	properties->transformW *= Matrix::CreateFromYawPitchRoll(objRotation);
	properties->transformW *= Matrix::CreateTranslation(tmpTranslation);
	properties->transformW = properties->transformW.Transpose();
	properties->transformWInvT = InverseTranspose(properties->transformW);

	//projection view calculation
	properties->transformH = camManager->viewMatrix;
	properties->transformH *= camManager->projectionMatrix;
	properties->transformH.Transpose(properties->transformH);

	if (collisionEnabled) {
		boxCollider.Center = tmpTranslation;
		sphereCollider.Center = tmpTranslation;
	}
}

//apply all transformations to object, then draw it
void GameComponent::Draw(ID3D11DeviceContext* context) 
{
	D3D11_MAPPED_SUBRESOURCE res = {};

	context->VSSetConstantBuffers(0, 1, &objectPropsBuffer);
	context->Map(objectPropsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

	auto dataPtr = reinterpret_cast<float*>(res.pData);
	memcpy(dataPtr, properties, sizeof(objectProps));

	context->Unmap(objectPropsBuffer, 0);

	if (type == Strip)
		TriangleComponent::DrawLine(context);
	else {
		if (m_texture != nullptr)
			context->PSSetShaderResources(0, 1, m_texture.GetAddressOf());
		TriangleComponent::DrawTriangle(context);
	}
}

//SHADOWS
void GameComponent::DrawShadow(ID3D11DeviceContext* context)
{
	D3D11_MAPPED_SUBRESOURCE res = {};
	context->VSSetConstantBuffers(0, 1, &objectPropsBuffer);
	context->Map(objectPropsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

	auto dataPtr = reinterpret_cast<float*>(res.pData);
	memcpy(dataPtr, properties, sizeof(objectProps));

	context->Unmap(objectPropsBuffer, 0);

	DrawShadowTriangle(context);
}
//END

void GameComponent::Reload() {
	scale = Vector3::One;
	rotation = rotation.CreateFromYawPitchRoll(Vector3(0.0f, 0.0f, 0.0f));
	translation = Vector3::Zero;
}

//sphere subdivision
void GameComponent::SphereSubdivide(std::vector<Vertex>* points, std::vector<int>* indexes)
{
	std::vector<Vertex> pointsTmp;
	std::vector<int> indexesTmp;
	int startIndex = (int) (*points).size();
	int index1 = -1, index2 = -1, index3 = -1, indTmp = -1;
	int curSize = (int) (*points).size();

	for (auto point : *points)
		pointsTmp.push_back(point);

	for (UINT i = 0; i < (*indexes).size() / 3; ++i) {

		Vertex point1 = { findCenter((*points)[(*indexes)[i * 3]].location, (*points)[(*indexes)[i * 3 + 1]].location), (*points)[(*indexes)[i * 3 + 2]].color };
		index1 = curSize;

		indTmp = CheckForUnique(points, point1, startIndex);

		if (indTmp == -1) {
			pointsTmp.push_back(point1);
			++curSize;
		}
		else
			index1 = indTmp;

		Vertex point2 = { findCenter((*points)[(*indexes)[i * 3]].location, (*points)[(*indexes)[i * 3 + 2]].location), (*points)[(*indexes)[i * 3 + 1]].color };
		index2 = curSize;

		indTmp = CheckForUnique(points, point2, startIndex);

		if (indTmp == -1) {
			pointsTmp.push_back(point2);
			++curSize;
		}
		else
			index2 = indTmp;

		Vertex point3 = { findCenter((*points)[(*indexes)[i * 3 + 1]].location, (*points)[(*indexes)[i * 3 + 2]].location), (*points)[(*indexes)[i * 3]].color };
		index3 = curSize;

		indTmp = CheckForUnique(points, point3, startIndex);

		if (indTmp == -1) {
			pointsTmp.push_back(point3);
			++curSize;
		}
		else
			index3 = indTmp;


		indexesTmp.push_back((*indexes)[i * 3]);
		indexesTmp.push_back(index1);
		indexesTmp.push_back(index2);

		indexesTmp.push_back((*indexes)[i * 3 + 1]);
		indexesTmp.push_back(index3);
		indexesTmp.push_back(index1);

		indexesTmp.push_back((*indexes)[i * 3 + 2]);
		indexesTmp.push_back(index2);
		indexesTmp.push_back(index3);

		indexesTmp.push_back(index1);
		indexesTmp.push_back(index3);
		indexesTmp.push_back(index2);
	}
	(*points).clear();
	(*indexes).clear();
	*points = pointsTmp;
	*indexes = indexesTmp;
}

Vector4 GameComponent::findCenter(const Vector4& point1, const Vector4& point2) {
	return Vector4((point1.x + point2.x) / 2.0f, (point1.y + point2.y) / 2.0f, (point1.z + point2.z) / 2.0f, 1.0f);
}

//check for unique sphere vertices
int GameComponent::CheckForUnique(const std::vector<Vertex>* points, Vertex pointNew, int startInd) {
	for (UINT i = startInd; i < (UINT) (*points).size(); ++i) {
		if ((*points)[i].location.x == pointNew.location.x && (*points)[i].location.y == pointNew.location.y && (*points)[i].location.z == pointNew.location.z)
			return i;
	}
	return -1;
}

void GameComponent::PointNormalize(Vertex& point) {	float tmp = (float) std::sqrt(std::pow(point.location.x, 2) + std::pow(point.location.y, 2) + std::pow(point.location.z, 2));	point.location.x /= tmp;	point.location.y /= tmp;	point.location.z /= tmp;
}

//load model from file
bool GameComponent::LoadModel(const std::string& filePath)
{
	Assimp::Importer importer;

	const aiScene* pScene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

	if (pScene == nullptr)
		return false;

	this->ProcessNode(pScene->mRootNode, pScene);

	return true;
}

void GameComponent::ProcessNode(aiNode* node, const aiScene* scene)
{
	for (UINT i = 0; i < node->mNumMeshes; ++i) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		this->ProcessMesh(mesh, scene);
	}

	for (UINT i = 0; i < node->mNumChildren; ++i)
		this->ProcessNode(node->mChildren[i], scene);
}

void GameComponent::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	points = new std::vector<Vertex>;
	indexes = new std::vector<int>;
	for (UINT i = 0; i < mesh->mNumVertices; ++i) {
		Vertex vertex;
		vertex.location.x = mesh->mVertices[i].x;
		vertex.location.y = mesh->mVertices[i].y;
		vertex.location.z = mesh->mVertices[i].z;
		vertex.location.w = 1.0f;

		if (mesh->mTextureCoords[0]) {
			vertex.texCoord.x = (float)mesh->mTextureCoords[0][i].x;
			vertex.texCoord.y = (float)mesh->mTextureCoords[0][i].y;
		}

		(*points).push_back(vertex);
	}

	for (UINT i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];

		for (UINT j = 0; j < face.mNumIndices; ++j)
			(*indexes).push_back(face.mIndices[j]);
	}
}

//load texture from file
bool GameComponent::LoadTexture(Microsoft::WRL::ComPtr<ID3D11Device> device, const wchar_t* texturePath)
{
	CD3D11_TEXTURE2D_DESC textureDesc(DXGI_FORMAT_R8G8B8A8_UNORM, 512, 512);

	CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D, textureDesc.Format);

	DirectX::CreateWICTextureFromFile(device.Get(), texturePath, resource.GetAddressOf(), m_texture.GetAddressOf());
	//DebugBreak();
	HRESULT hr = device->CreateShaderResourceView(resource.Get(), &srvDesc, m_texture.GetAddressOf());
	if (hr == S_OK)
		return true;
	else return false;
}
