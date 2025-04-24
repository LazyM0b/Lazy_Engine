#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/material.h>	
#include <WICTextureLoader.h>

#include "TriangleComponent.h"
#include "ShadowsComponent.h"
#include "DirectXCollision.h"
#include "SimpleMath.h"
#include "DisplayWin32.h"
#include "CameraManager.h"

using namespace DirectX::SimpleMath;

enum MaterialTypes 
{
	Rubber,
	Plastic,
	Metal
};

enum CollisionTypes
{
	NoCollision,
	Static,
	Dynamic
};

class GameComponent: public TriangleComponent {
public:

	GameComponent();
	GameComponent(Microsoft::WRL::ComPtr<ID3D11Device> device, const GameComponent& other);
	void Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device, MeshTypes type);
	void Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device, MeshTypes type, MaterialTypes matType, std::vector<Vector4> colors, UINT detailsLVL = 3);
	void Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device, MaterialTypes matType, const std::string& modelPath, const wchar_t* texturePath);
	void Update(CameraManager* camManager);
	void Draw(ID3D11DeviceContext* context, ShadowMap* shadowMap);
	void DrawShadow(ID3D11DeviceContext* context);
	void Reload();
	void DestroyResources();
	void PointNormalize(Vertex& point);
	void SphereSubdivide(std::vector<Vertex>& points, std::vector<int>& indeces);
	Vector4 findCenter(const Vector4& point1, const Vector4& point2);
	int CheckForUnique(const std::vector<Vertex>& points, Vertex pointNew, int startInd);
	
	bool LoadModel(const std::string& filePath);
	void ProcessNode(aiNode* node, const aiScene* scene);
	void ProcessMesh(aiMesh* mesh, const aiScene* scene);

	bool LoadTexture(Microsoft::WRL::ComPtr<ID3D11Device> device, const wchar_t* texturePath);
	//ID3D11ShaderResourceView** GetSRV() { return &m_texture; };

	bool collisionEnabled = false;
	bool isMovable = false;
	bool isPlayerControllable = false;
	UINT pointsCnt;
	DirectX::BoundingBox boxCollider;
	DirectX::BoundingSphere sphereCollider;
	CollisionTypes collisionType;
	float speed = 0.0f;
	float initialSpeed = 0.0f;
	float speedMax = 0.0f;
	float dashCD = 3.0f;
	float period;
	float rotationAngle;
	Microsoft::WRL::ComPtr <ID3D11Resource> resource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
	Vector3 distanceToParent;
	Vector3 scale;
	Vector3 dimensions;
	Quaternion rotation;
	Vector3 translation;
	Vector3 impulse;
	Vector3 velocity;
	Matrix positionL;
	objectProps* properties;

	GameComponent* parent;
};