#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <vector>

#include "SimpleMath.h"
#include "ShadersComponent.h"

using namespace DirectX::SimpleMath;

const UINT BITONIC_BLOCK_SIZE = 512;
const UINT TRANSPOSE_BLOCK_SIZE = 16;

struct Particle
{
	Vector4 color;
	Vector4 colorDelta;
	Vector4 pos;
	Vector4 prevPos;
	Vector3 velocity;
	Vector3 acceleration;
	Vector2 size;
	Vector2 sizeDelta;
	float lifeTime;
	float weight;
};

struct ParticleWorldProps
{
	Matrix ViewMatrix;
	Matrix ProjectionMatrix;
};

struct SortData
{
	UINT index;
	float distance;
};

struct DeadData
{
	UINT index;
};

struct CB
{
	UINT iLevel;
	UINT iLevelMask;
	UINT iWidth;
	UINT iHeight;
};

struct ParticleSystemProps
{
	int groupsCnt;
	UINT maxParticles;
	float deltaTime;
	float initialLifeTime;
};

enum ParticleSystemType: UINT8
{
	Fountain,
	Fire,
	Rain
};

struct DrawArgs
{
	UINT IndexCountPerInstance;
	UINT InstanceCount;
	UINT StartIndexLocation;
	int BaseVertexLocation;
	UINT StartInstanceLocation;
};

class ParticleSystem
{
public:
	ParticleSystem(ParticleSystemType type, UINT particlesNum, Vector4 spawnPoint);
	virtual void Update(float deltaTime);
	virtual void Render();
	virtual int Emit(int numParticles);
	virtual void InitSystem();
	virtual void KillSystem();

//protected:
	virtual void InitParticle(int index);
	ParticleSystemType systemType;
	std::vector <Particle>* particleList;
	int maxParticles;
	int curSecondParticles;
	int numParticles;
	Vector4 origin;
	Vector3 force;
	float emissionRate;
	float timeFromEmission;

	//particle system lifetime attributes
	float timeFromStart;
	float maxTime;
};

class ParticleSystemComponent
{
public:
	std::vector<ParticleSystem> particleSystems;
	void InitializeSystem(Microsoft::WRL::ComPtr<ID3D11Device> device, ID3D11DeviceContext* context, size_t index);
	void AddParticleSystem(Microsoft::WRL::ComPtr<ID3D11Device> device, ID3D11DeviceContext* context, ParticleSystemType type, UINT particlesNum, Vector4 spawnPoint);
	void PrepareParticleSystems(ID3D11DeviceContext* context, size_t index);
	void EmitParticles(float DeltaTime, ID3D11DeviceContext* context);
	void UpdateSystems(float DeltaTime, ID3D11DeviceContext* context);
	void ConsumeParticles(ID3D11DeviceContext* context);
	void Draw(ID3D11DeviceContext* context);
	void SortParticles(ID3D11DeviceContext* context, ShadersComponent* shaders);
	void SetConstants(ID3D11DeviceContext* context, UINT iLevel, UINT iLevelMask, UINT iWidth, UINT iHeight);

	std::vector<ParticleSystem*> systems;
	std::vector <std::vector<int>> indexList;

	std::vector<ID3D11Buffer*> particlesBuffers;
	std::vector<ID3D11ShaderResourceView*> particlesSRV;
	std::vector<ID3D11UnorderedAccessView*> particlesUAV;

	std::vector<ID3D11Buffer*> sortDataBuffers;
	std::vector<ID3D11ShaderResourceView*> sortDataSRV;
	std::vector<ID3D11UnorderedAccessView*> sortDataUAV;
	std::vector<ID3D11UnorderedAccessView*> sortDataAppendUAV;

	std::vector<ID3D11Buffer*> deadDataBuffers;
	std::vector<ID3D11UnorderedAccessView*> deadDataUAV;

	std::vector<ParticleSystemProps> particlesProperties;
	std::vector<ID3D11Buffer*> particlesPropsBuffers;

	std::vector<ID3D11Buffer*> indexBuffers;
	std::vector<ID3D11Buffer*> argsBuffers;

	ID3D11Buffer* constSortBuffer;

	ID3D11Buffer* propsBuffer;
	ParticleWorldProps worldProperties;

	std::vector<UINT> particlesCnt;
	UINT particleStrides;
	UINT particleOffsets;
};
