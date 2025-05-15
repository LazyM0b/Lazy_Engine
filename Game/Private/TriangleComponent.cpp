#include "TriangleComponent.h"

TriangleComponent::TriangleComponent() {}

//all object buffers initialization
void TriangleComponent::Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device) {

	D3D11_BUFFER_DESC vertexBufDesc = {};
	vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufDesc.CPUAccessFlags = 0;
	vertexBufDesc.MiscFlags = 0;
	vertexBufDesc.StructureByteStride = 0;
	int i = sizeof(Vertex);
	vertexBufDesc.ByteWidth = sizeof(Vertex) * std::size(*points);

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = &(*points).front();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	device->CreateBuffer(&vertexBufDesc, &vertexData, &vertexBuffer);

	strides = sizeof(Vertex);
	offsets = 0;

	D3D11_BUFFER_DESC shadowVertexBufDesc = {};
	shadowVertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	shadowVertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	shadowVertexBufDesc.CPUAccessFlags = 0;
	shadowVertexBufDesc.MiscFlags = 0;
	shadowVertexBufDesc.StructureByteStride = 0;
	shadowVertexBufDesc.ByteWidth = sizeof(shadowVertex) * std::size((*shadowPoints));

	D3D11_SUBRESOURCE_DATA shadowVertexData = {};
	shadowVertexData.pSysMem = &(*shadowPoints).front();
	shadowVertexData.SysMemPitch = 0;
	shadowVertexData.SysMemSlicePitch = 0;

	device->CreateBuffer(&shadowVertexBufDesc, &shadowVertexData, &shadowVertexBuffer);

	shadowStrides = sizeof(shadowVertex);
	shadowOffsets = 0;


	if (type != Strip) {
		//index buffer initialization
		D3D11_BUFFER_DESC indexBufDesc = {};
		indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufDesc.CPUAccessFlags = 0;
		indexBufDesc.MiscFlags = 0;
		indexBufDesc.StructureByteStride = 0;
		indexBufDesc.ByteWidth = sizeof(int) * std::size((*indexes));

		D3D11_SUBRESOURCE_DATA indexData = {};
		indexData.pSysMem = &(*indexes).front();
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		device->CreateBuffer(&indexBufDesc, &indexData, &indexBuffer);
	}

	D3D11_BUFFER_DESC objectBufDesc = {};
	objectBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	objectBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	objectBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	objectBufDesc.MiscFlags = 0;
	objectBufDesc.StructureByteStride = 0;
	objectBufDesc.ByteWidth = sizeof(objectProps);

	device->CreateBuffer(&objectBufDesc, 0, &objectPropsBuffer);
}

void TriangleComponent::DrawTriangle(ID3D11DeviceContext* context)
{
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &strides, &offsets);

	context->DrawIndexed((*indexes).size(), 0, 0);
}

//SHADOWS
void TriangleComponent::DrawShadowTriangle(ID3D11DeviceContext* context)
{
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	context->IASetVertexBuffers(0, 1, &shadowVertexBuffer, &shadowStrides, &shadowOffsets);

	context->DrawIndexed((*indexes).size(), 0, 0);
}
//END

void TriangleComponent::DrawLine(ID3D11DeviceContext* context)
{
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &strides, &offsets);

	context->Draw((*points).size(), 0);
}

void TriangleComponent::ComputeNormal(const Vector3& p1, const Vector3& p2, const Vector3& p3, Vector3& outN)
{
	Vector3 u = p2 - p1;
	Vector3 v = p3 - p1;
	outN = u.Cross(v);
}
