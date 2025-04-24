#pragma once

#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

struct Material {
	Vector4 ambient;
	Vector4 diffuse;
	Vector4 specular;
	Vector4 reflection;
};

struct DirectionalLight {
	Vector4 ambient;
	Vector4 diffuse;
	Vector4 specular;
	Vector4 direction;
};

struct PointLight {
	Vector4 ambient;
	Vector4 diffuse;
	Vector4 specular;

	Vector4 position;
	Vector4 attenuation;
};

struct LightningData {
	DirectionalLight dirLight;
	PointLight pointLights[512];
	Vector3 eyePos;
	int lightsNum;
};

static Matrix InverseTranspose(Matrix inputMatrix) {
	inputMatrix.Translation(Vector3::Zero);
	inputMatrix.Invert(inputMatrix);
	inputMatrix.Transpose(inputMatrix);

	return inputMatrix;
}