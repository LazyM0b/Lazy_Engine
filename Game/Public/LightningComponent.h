#pragma once

#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

struct Material {
	Vector4 diffuse;
	Vector4 specular;
};

struct DirectionalLight {
	Vector4 ambient;
	Vector4 diffuse;
	Vector4 specular;
	Vector4 direction;
};

struct PointLight {
	Vector4 diffuse;
	Vector4 specular;

	Vector4 position;
	Vector4 attenuation;
};

struct SpotLight {
	Vector4 diffuse;
	Vector4 specular;

	Vector4 position;
	Vector4 attenuation;
	Vector4 cone;
};

struct LightningData {
	DirectionalLight dirLight;
	PointLight pointLights[400];
	SpotLight spotLights[400];
	Vector4 eyePos;
	Matrix transformHInv;
	Vector4 Data;
};

static Matrix InverseTranspose(Matrix inputMatrix) {
	inputMatrix.Translation(Vector3::Zero);
	inputMatrix.Invert(inputMatrix);
	inputMatrix.Transpose(inputMatrix);

	return inputMatrix;
}