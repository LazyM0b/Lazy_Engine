
#include "GameComponent.h"
#include "DeferredRenderingComponent.h"

void DeferredSystem::Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device, UINT clientWidth, UINT clientHeight, UINT mapSize)
{
	D3D11_BLEND_DESC blendDesc;

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HRESULT res = device->CreateBlendState(&blendDesc, &blendState);

	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = clientWidth;
	textureDesc.Height = clientHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	for (int i = 0; i < GBufferSize; ++i)
	{
		res = device->CreateTexture2D(&textureDesc, nullptr, &opaqueBuffer[i]);
		res = device->CreateRenderTargetView(opaqueBuffer[i], nullptr, &opaqueRenderView[i]); // second parameter may be not nullptr if it's not for backbuffer
		res = device->CreateShaderResourceView(opaqueBuffer[i], &srvDesc, &opaqueSRV[i]);
	}

	D3D11_BUFFER_DESC pointsBufDesc = {};
	pointsBufDesc.Usage = D3D11_USAGE_DEFAULT;
	pointsBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	pointsBufDesc.CPUAccessFlags = 0;
	pointsBufDesc.MiscFlags = 0;
	pointsBufDesc.StructureByteStride = 0;
	pointsBufDesc.ByteWidth = sizeof(UINT) * clientWidth * clientHeight;

	points = new std::vector<UINT>;
	for (UINT i = 0; i < clientHeight * clientWidth; ++i)
		points->push_back(i);

	D3D11_SUBRESOURCE_DATA pointsData = {};
	pointsData.pSysMem = &points->front();
	pointsData.SysMemPitch = 0;
	pointsData.SysMemSlicePitch = 0;

	res = device->CreateBuffer(&pointsBufDesc, &pointsData, &pointsBuffer);

	for (int i = 0; i < 100; ++i)
		transpObjects.push_back(std::vector<GameComponent*>(mapSize * 2));
}

void DeferredSystem::DrawOpaque(ID3D11DeviceContext* context, std::vector<GameComponent*> objects)
{
	for (GameComponent* object : objects) {
		if (!object->isTransparent && object->shouldBeRendered)
			object->Draw(context);
	}
}

void DeferredSystem::DrawLighting(ID3D11DeviceContext* context, UINT width, UINT height)
{
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	UINT strides = sizeof(UINT);
	UINT offsets = 0;
	context->IASetVertexBuffers(0, 1, &pointsBuffer, &strides, &offsets);

	context->Draw(width * height, 0);
}

void DeferredSystem::DrawTransparent(ID3D11DeviceContext* context, std::vector<GameComponent*> objects, UINT mapSize)
{
	for (GameComponent* object : objects)
	{
		if (object->isTransparent && object->shouldBeRendered)
		{
			Matrix transformMat = object->properties->transformW.Transpose();
			transformMat *= object->properties->transformH.Transpose();
			Vector4 nearestPoint = { 0.0f, 0.0f, -1.0f, 1.0f };
			nearestPoint = Vector4::Transform(nearestPoint, transformMat);

			for (int i = 0; i < 100; ++i)
			{
				int k = (int)nearestPoint.z;
				if (k < 0.0f)
					break;

				if (transpObjects[i][k] == NULL) {
					transpObjects[i][k] = object;
					break;
				}
			}
		}
	}
	for (int i = mapSize - 1; i >= 0; --i)
	{
		for (int j = 0; j < 100; ++j)
		{
			if (transpObjects[j][i] == NULL)
			{
				break;
			}
			else
			{
				transpObjects[j][i]->Draw(context);
				transpObjects[j][i] = NULL;
			}
		}
	}
}

void DeferredSystem::PrepareFrame()
{
}