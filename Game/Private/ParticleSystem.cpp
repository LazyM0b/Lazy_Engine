
#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(ParticleSystemType type, UINT particlesNum, Vector4 spawnPoint):  systemType(type), maxParticles(particlesNum), origin(spawnPoint) 
{
	emissionRate = 10000.0f;
	maxTime = 5.0f;
	SpawnRange = 100.0f;
}

void ParticleSystem::Update(float deltaTime)
{

}

void ParticleSystem::Render()
{

}

void ParticleSystem::KillSystem()
{

}

void ParticleSystem::InitSystem()
{
	particleList = new std::vector<Particle>;
	for (int i = 0; i < maxParticles; ++i)
	{
		particleList->push_back(Particle());
		InitParticle(i);
	}
}

void ParticleSystem::InitParticle(int index)
{
	Particle* particle = &(*particleList)[index];
	particle->origin = Vector4((rand() % int(SpawnRange * 2) - SpawnRange) + origin.x, (rand() % int(SpawnRange * 2) - SpawnRange) + origin.y, (rand() % int(SpawnRange * 2) - SpawnRange) + origin.z, 1.0f);
	particle->pos = particle->origin;
	particle->initialVelocity = Vector3((rand() % 80 - 40) * 1.0f, (rand() % 10 + 10) * 1.0f, (rand() % 80 - 40) * 1.0f);
	particle->velocity = particle->initialVelocity;
	particle->acceleration = Vector3(0.0f, -0.1f, 0.0f);
	particle->color = Vector4((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f);
	particle->lifeTime = maxTime;
	float tmp = (rand() % 50 + 50) / 100.0f * 10.0f;
	particle->size = Vector2(tmp, tmp);
}

int ParticleSystem::Emit(int numParticles)
{

	//must return info about dead particles (count or maybe index?)
	return 0;
}

UINT ParticleSystemComponent::AddParticleSystem(Microsoft::WRL::ComPtr<ID3D11Device> device, ID3D11DeviceContext* context, ParticleSystemType type, UINT particlesNum, Vector4 spawnPoint)
{
	systems.push_back(new ParticleSystem(type, particlesNum, spawnPoint));
	particlesProperties.push_back(ParticleSystemProps());
	particlesCnt.push_back(0);
	return InitializeSystem(device, context, systems.size() - 1);
}

UINT ParticleSystemComponent::InitializeSystem(Microsoft::WRL::ComPtr<ID3D11Device> device, ID3D11DeviceContext* context, size_t index)
{
	systems[index]->InitSystem();
	particlesProperties[index].groupsCnt = (systems[index]->maxParticles % 768 != 0) ? ((systems[index]->maxParticles / 768) + 1) : (systems[index]->maxParticles / 768);
	particlesProperties[index].groupsCnt = ceil(pow(particlesProperties[index].groupsCnt, 0.5f));
	particlesProperties[index].maxParticles = systems[index]->maxParticles;
	//particlesProperties[index].originPos = systems[index]->origin;

	D3D11_BUFFER_DESC particlePropsBufDesc = {};
	particlePropsBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	particlePropsBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	particlePropsBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	particlePropsBufDesc.MiscFlags = 0;
	particlePropsBufDesc.StructureByteStride = 0;
	particlePropsBufDesc.ByteWidth = sizeof(ParticleSystemProps);

	D3D11_SUBRESOURCE_DATA propsData = {};
	propsData.pSysMem = &particlesProperties[index];
	propsData.SysMemPitch = 0;
	propsData.SysMemSlicePitch = 0;

	ID3D11Buffer* particlesPropsBuffer;
	device->CreateBuffer(&particlePropsBufDesc, &propsData, &particlesPropsBuffer);
	particlesPropsBuffers.push_back(particlesPropsBuffer);

	//D3D11_MAPPED_SUBRESOURCE res = {};
	context->CSSetConstantBuffers(0, 1, &particlesPropsBuffers[index]);
	/*context->Map(particlesPropsBuffers[index], 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

	auto dataPtr = reinterpret_cast<float*>(res.pData);
	memcpy(dataPtr, &particlesProperties[index], sizeof(ParticleSystemProps));

	context->Unmap(particlesPropsBuffers[index], 0);*/

	//Init particles buffer, Shader resource view and unordered access view
	D3D11_BUFFER_DESC particlesBufDesc = {};
	particlesBufDesc.Usage = D3D11_USAGE_DEFAULT;
	particlesBufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	particlesBufDesc.CPUAccessFlags = 0;
	particlesBufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	particlesBufDesc.StructureByteStride = sizeof(Particle);
	particlesBufDesc.ByteWidth = sizeof(Particle) * systems[index]->maxParticles;

	D3D11_SUBRESOURCE_DATA particlesData = {};
	particlesData.pSysMem = &(*systems[index]->particleList).front();
	particlesData.SysMemPitch = 0;
	particlesData.SysMemSlicePitch = 0;

	ID3D11Buffer* particlesBuffer;

	device->CreateBuffer(&particlesBufDesc, &particlesData, &particlesBuffer);

	particlesBuffers.push_back(particlesBuffer);

	particleStrides = sizeof(Particle);
	particleOffsets = 0;

	ID3D11ShaderResourceView* particleSRV;
	device->CreateShaderResourceView(particlesBuffers[index], nullptr, &particleSRV);
	particlesSRV.push_back(particleSRV);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer = D3D11_BUFFER_UAV{
		0,
		(UINT)systems[0]->maxParticles,
		0
	};

	ID3D11UnorderedAccessView* particleUAV;
	device->CreateUnorderedAccessView(particlesBuffers[index], &uavDesc, &particleUAV);
	particlesUAV.push_back(particleUAV);


	//Fill index list
	indexList.push_back(std::vector<int>());
	for (int i = 0; i < systems[index]->maxParticles; ++i)
	{
		indexList[index].push_back(8 * i + 0);
		indexList[index].push_back(8 * i + 1);
		indexList[index].push_back(8 * i + 2);

		indexList[index].push_back(8 * i + 1);
		indexList[index].push_back(8 * i + 0);
		indexList[index].push_back(8 * i + 3);

		indexList[index].push_back(8 * i + 2);
		indexList[index].push_back(8 * i + 1);
		indexList[index].push_back(8 * i + 0);

		indexList[index].push_back(8 * i + 3);
		indexList[index].push_back(8 * i + 0);
		indexList[index].push_back(8 * i + 1);


		indexList[index].push_back(8 * i + 4);
		indexList[index].push_back(8 * i + 5);
		indexList[index].push_back(8 * i + 6);

		indexList[index].push_back(8 * i + 5);
		indexList[index].push_back(8 * i + 4);
		indexList[index].push_back(8 * i + 7);

		indexList[index].push_back(8 * i + 6);
		indexList[index].push_back(8 * i + 5);
		indexList[index].push_back(8 * i + 4);

		indexList[index].push_back(8 * i + 7);
		indexList[index].push_back(8 * i + 4);
		indexList[index].push_back(8 * i + 5);
	}

	//Init indexes buffer
	D3D11_BUFFER_DESC indexBufDesc = {};
	indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufDesc.CPUAccessFlags = 0;
	indexBufDesc.MiscFlags = 0;
	indexBufDesc.StructureByteStride = 0;
	indexBufDesc.ByteWidth = sizeof(int) * indexList[index].size();

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = &indexList[index].front();
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	ID3D11Buffer* indexBuffer;

	device->CreateBuffer(&indexBufDesc, &indexData, &indexBuffer);

	indexBuffers.push_back(indexBuffer);

	D3D11_BUFFER_DESC objectBufDesc = {};
	objectBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	objectBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	objectBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	objectBufDesc.MiscFlags = 0;
	objectBufDesc.StructureByteStride = 0;
	objectBufDesc.ByteWidth = sizeof(ParticleWorldProps);

	device->CreateBuffer(&objectBufDesc, 0, &propsBuffer);

	//Init sort (particles depths) list - buffer, SRV and UAV
	D3D11_BUFFER_DESC particleDepthBufDesc = {};
	particleDepthBufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	particleDepthBufDesc.Usage = D3D11_USAGE_DEFAULT;
	particleDepthBufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	particleDepthBufDesc.CPUAccessFlags = 0;
	particleDepthBufDesc.StructureByteStride = sizeof(SortData);
	particleDepthBufDesc.ByteWidth = systems[index]->maxParticles * sizeof(SortData);

	for (int i = 0; i < systems[index]->maxParticles; ++i)
	{
		depthsData.push_back(SortData(i, 0.f));
	}

	D3D11_SUBRESOURCE_DATA initSortData = {};
	initSortData.pSysMem = &depthsData.front();
	initSortData.SysMemPitch = 0;
	initSortData.SysMemSlicePitch = 0;

	ID3D11Buffer* sortDataBuffer;

	device->CreateBuffer(&particleDepthBufDesc, &initSortData, &sortDataBuffer);
	sortDataBuffers.push_back(sortDataBuffer);

	ID3D11Buffer* sortDataConsumeBuffer;

	device->CreateBuffer(&particleDepthBufDesc, &initSortData, &sortDataConsumeBuffer);
	sortDataConsumeBuffers.push_back(sortDataBuffer);

	ID3D11ShaderResourceView* sortSRV;
	device->CreateShaderResourceView(sortDataBuffers[index], nullptr, &sortSRV);
	sortDataSRV.push_back(sortSRV);

	D3D11_UNORDERED_ACCESS_VIEW_DESC sortUavDesc = {};
	sortUavDesc.Format = DXGI_FORMAT_UNKNOWN;
	sortUavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	sortUavDesc.Buffer = D3D11_BUFFER_UAV{
		0,
		(UINT)systems[0]->maxParticles,
		D3D11_BUFFER_UAV_FLAG_APPEND
	};

	ID3D11UnorderedAccessView* sortUAV;
	device->CreateUnorderedAccessView(sortDataBuffers[index], &sortUavDesc, &sortUAV);
	sortDataAppendUAV.push_back(sortUAV);


	ID3D11UnorderedAccessView* sortUAV1;
	device->CreateUnorderedAccessView(sortDataConsumeBuffers[index], &sortUavDesc, &sortUAV1);
	sortDataConsumeUAV.push_back(sortUAV1);


	sortUavDesc.Buffer = D3D11_BUFFER_UAV{
		0,
		(UINT)systems[0]->maxParticles,
		0
	};

	device->CreateUnorderedAccessView(sortDataBuffers[index], &sortUavDesc, &sortUAV);
	sortDataUAV.push_back(sortUAV);

	//Create sort shader constant buffer
	D3D11_BUFFER_DESC constant_buffer_desc = {};
	constant_buffer_desc.ByteWidth = sizeof(CB);
	constant_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constant_buffer_desc.CPUAccessFlags = 0;

	device->CreateBuffer(&constant_buffer_desc, nullptr, &constSortBuffer);

	//Init dead (free particles indexes) list - buffer and UAV
	D3D11_BUFFER_DESC particleDeadBufDesc = {};
	particleDeadBufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	particleDeadBufDesc.Usage = D3D11_USAGE_DEFAULT;
	particleDeadBufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	particleDeadBufDesc.CPUAccessFlags = 0;
	particleDeadBufDesc.StructureByteStride = sizeof(DeadData);
	particleDeadBufDesc.ByteWidth = systems[index]->maxParticles * sizeof(DeadData);

	for (int i = 0; i < systems[index]->maxParticles; ++i)
	{
		deadData.push_back(DeadData(i));
	}

	D3D11_SUBRESOURCE_DATA initDeadData = {};
	initDeadData.pSysMem = &deadData.front();
	initDeadData.SysMemPitch = 0;
	initDeadData.SysMemSlicePitch = 0;

	ID3D11Buffer* deadDataBuffer;

	device->CreateBuffer(&particleDeadBufDesc, &initDeadData, &deadDataBuffer);
	deadDataBuffers.push_back(deadDataBuffer);

	D3D11_UNORDERED_ACCESS_VIEW_DESC deadUavDesc = {};
	deadUavDesc.Format = DXGI_FORMAT_UNKNOWN;
	deadUavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	deadUavDesc.Buffer = D3D11_BUFFER_UAV{
		0,
		(UINT)systems[0]->maxParticles,
		D3D11_BUFFER_UAV_FLAG_APPEND
	};

	ID3D11UnorderedAccessView* deadUAV;
	device->CreateUnorderedAccessView(deadDataBuffers[index], &deadUavDesc, &deadUAV);
	deadDataUAV.push_back(deadUAV);

	context->CSSetUnorderedAccessViews(3, 1, &deadDataUAV[index], 0);

	return particlesProperties[index].groupsCnt;
}

void ParticleSystemComponent::PrepareParticleSystems(ID3D11DeviceContext* context, size_t index)
{

}

UINT ParticleSystemComponent::EmitParticles(float DeltaTime, ID3D11DeviceContext* context, size_t index)
{
	//if (isEmitted)
	//	return 0;
		if (particlesCnt[index] == systems[index]->emissionRate * systems[index]->maxTime)
			return 0;

		float deltaParticlesCnt = systems[index]->emissionRate * DeltaTime;
		if (particlesCnt[index] + deltaParticlesCnt > systems[index]->emissionRate * systems[index]->maxTime)
			deltaParticlesCnt = systems[index]->emissionRate * systems[index]->maxTime - particlesCnt[index];
		particlesProperties[index].deltaTime = DeltaTime;
		particlesProperties[index].maxParticles = deltaParticlesCnt;
		particlesProperties[index].groupsCnt = (particlesProperties[index].maxParticles % 768 != 0) ? ((particlesProperties[index].maxParticles / 768) + 1) : (particlesProperties[index].maxParticles / 768);
		particlesProperties[index].groupsCnt = ceil(pow(particlesProperties[index].groupsCnt, 0.5f));
		particlesCnt[index] += particlesProperties[index].maxParticles;
		//isEmitted = true;

		context->CSSetUnorderedAccessViews(1, 1, &sortDataAppendUAV[index], 0);
		context->CSSetUnorderedAccessViews(4, 1, &deadDataUAV[index], 0);

	return particlesProperties[index].groupsCnt;
}

UINT ParticleSystemComponent::UpdateSystems(float DeltaTime, ID3D11DeviceContext* context, size_t index)
{
	particlesProperties[index].deltaTime = DeltaTime;
	particlesProperties[index].initialLifeTime = systems[index]->maxTime;
	particlesProperties[index].maxParticles = particlesCnt[index];
	particlesProperties[index].groupsCnt = (particlesProperties[index].maxParticles % 768 != 0) ? ((particlesProperties[index].maxParticles / 768) + 1) : (particlesProperties[index].maxParticles / 768);
	particlesProperties[index].groupsCnt = ceil(pow(particlesProperties[index].groupsCnt, 0.5f));

	D3D11_MAPPED_SUBRESOURCE res = {};
	context->Map(particlesPropsBuffers[index], 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

	auto dataPtr = reinterpret_cast<float*>(res.pData);
	memcpy(dataPtr, &particlesProperties[index], sizeof(ParticleSystemProps));

	context->Unmap(particlesPropsBuffers[index], 0);

	context->CSSetConstantBuffers(0, 1, &particlesPropsBuffers[index]);
	context->CSSetConstantBuffers(1, 1, &propsBuffer);
	context->CSSetUnorderedAccessViews(0, 1, &particlesUAV[index], 0);
	context->CSSetShaderResources(0, 1, &sortDataSRV[index]);

	return particlesProperties[index].groupsCnt;
}

void ParticleSystemComponent::ConsumeParticles(ID3D11DeviceContext* context, size_t index)
{
	context->CSSetConstantBuffers(0, 1, &particlesPropsBuffers[index]);
	context->CSSetUnorderedAccessViews(0, 1, &particlesUAV[index], 0);
	context->CSSetUnorderedAccessViews(1, 1, &sortDataAppendUAV[index], 0);
	context->CSSetUnorderedAccessViews(2, 1, &sortDataConsumeUAV[index], 0);
	context->CSSetUnorderedAccessViews(3, 1, &deadDataUAV[index], 0);
}

void ParticleSystemComponent::Draw(Microsoft::WRL::ComPtr<ID3D11Device> device, ID3D11DeviceContext* context, size_t index)
{
	D3D11_MAPPED_SUBRESOURCE res = {};

	//printf("%d\n", particlesCnt[0]);
	context->VSSetConstantBuffers(0, 1, &propsBuffer);
	context->Map(propsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

	auto dataPtr = reinterpret_cast<float*>(res.pData);
	memcpy(dataPtr, &worldProperties, sizeof(ParticleWorldProps));

	context->Unmap(propsBuffer, 0);

	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (int i = 0; i < systems.size(); ++i)
	{
		context->IASetIndexBuffer(indexBuffers[i], DXGI_FORMAT_R32_UINT, 0);
		context->VSSetShaderResources(0, 1, &particlesSRV[i]);
		context->VSSetShaderResources(1, 1, &sortDataSRV[i]);

		D3D11_BUFFER_DESC argsBufferDesc;
		argsBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		argsBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		argsBufferDesc.CPUAccessFlags = 0;
		argsBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
		argsBufferDesc.StructureByteStride = sizeof(DrawArgs);
		argsBufferDesc.ByteWidth = sizeof(DrawArgs);

		DrawArgs args;
		args.IndexCountPerInstance = particlesProperties[index].maxParticles * 24;
		args.InstanceCount = 1;
		args.StartIndexLocation = 0;
		args.BaseVertexLocation = 0;
		args.StartInstanceLocation = 0;

		D3D11_SUBRESOURCE_DATA argsData;
		argsData.pSysMem = &args;

		ID3D11Buffer* argsBuffer;

		device->CreateBuffer(&argsBufferDesc, &argsData, &argsBuffer);
		argsBuffers.push_back(argsBuffer);

		/*DrawArgs args;
		args.IndexCountPerInstance = particlesCnt[i] * 24;
		args.InstanceCount = 1;
		args.StartIndexLocation = 0;
		args.BaseVertexLocation = 0;
		args.StartInstanceLocation = 0;*/

		/*D3D11_MAPPED_SUBRESOURCE res = {};
		context->Map(argsBuffers[i], 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

		auto dataPtr = reinterpret_cast<float*>(res.pData);
		memcpy(dataPtr, &args, sizeof(ParticleSystemProps));

		context->Unmap(argsBuffers[i], 0);*/

		context->DrawIndexedInstancedIndirect(argsBuffers[i], 0);//(UINT) argsBuffers[i] - (UINT) argsBuffers[0]);
		argsBuffers.pop_back();
	}
}

void ParticleSystemComponent::SortParticles(ID3D11DeviceContext* context, ShadersComponent* shaders)
{
	for (int i = 0; i < systems.size(); ++i)
	{
		UINT MaxParticlesCount = systems[i]->maxParticles;

		const UINT MATRIX_HEIGHT = MaxParticlesCount / BITONIC_BLOCK_SIZE;
		const UINT MATRIX_WIDTH = BITONIC_BLOCK_SIZE;

		// Sort the data
		// First sort the rows for the levels <= to the block size
		shaders->SortParticles(context);
		for (UINT level = 2; level <= BITONIC_BLOCK_SIZE; level = level * 2)
		{
			SetConstants(context, level, level, MATRIX_HEIGHT, MATRIX_WIDTH);

			// Sort the row data
			context->CSSetUnorderedAccessViews(0, 1, &sortDataUAV[i], nullptr);
			context->Dispatch(MATRIX_HEIGHT, 1, 1);
		}

		// Then sort the rows and columns for the levels > than the block size
		// Transpose. Sort the Columns. Transpose. Sort the Rows.
		for (UINT level = (BITONIC_BLOCK_SIZE * 2); level <= MaxParticlesCount; level = level * 2)
		{
			SetConstants(context, (level / BITONIC_BLOCK_SIZE), (level & ~MaxParticlesCount) / BITONIC_BLOCK_SIZE, MATRIX_WIDTH, MATRIX_HEIGHT);

			// Transpose the data from buffer 1 into buffer 2
			context->CSSetUnorderedAccessViews(0, 1, &sortDataUAV[i], nullptr);
			shaders->TransposeParticles(context);
			context->Dispatch(MATRIX_WIDTH / TRANSPOSE_BLOCK_SIZE, MATRIX_HEIGHT / TRANSPOSE_BLOCK_SIZE, 1);

			// Sort the transposed column data
			shaders->SortParticles(context);
			context->Dispatch(MaxParticlesCount / BITONIC_BLOCK_SIZE, 1, 1);

			SetConstants(context, BITONIC_BLOCK_SIZE, level, MATRIX_HEIGHT, MATRIX_WIDTH);

			// Transpose the data from buffer 2 back into buffer 1
			context->CSSetUnorderedAccessViews(0, 1, &sortDataUAV[i], nullptr);
			shaders->TransposeParticles(context);
			context->Dispatch(MATRIX_HEIGHT / TRANSPOSE_BLOCK_SIZE, MATRIX_WIDTH / TRANSPOSE_BLOCK_SIZE, 1);

			// Sort the row data
			shaders->SortParticles(context);
			context->Dispatch(MaxParticlesCount / BITONIC_BLOCK_SIZE, 1, 1);

			ID3D11UnorderedAccessView* pViewnullptr = nullptr;
			context->CSSetUnorderedAccessViews(0, 1, &pViewnullptr, nullptr);
		}
	}
}

void ParticleSystemComponent::SetConstants(ID3D11DeviceContext* context, UINT iLevel, UINT iLevelMask, UINT iWidth, UINT iHeight)
{
	CB cb = { iLevel, iLevelMask, iWidth, iHeight };
	context->UpdateSubresource(constSortBuffer, 0, nullptr, &cb, 0, 0);
	context->CSSetConstantBuffers(0, 1, &constSortBuffer);
}