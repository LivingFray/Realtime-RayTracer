#pragma once
#include <glm\glm.hpp>

//Due to the wonders of byte alignment pad everything to be 4 floats
struct Sphere {
	glm::vec3 pos;
	float radius;
	int material;
	float paddingA, paddingB, paddingC;
};

struct Plane {
	glm::vec3 pos;
	float paddingA;
	glm::vec3 norm;
	int material;
};

struct Triangle {
	glm::vec3 v1;
	float paddingA;
	glm::vec3 v2;
	float paddingB;
	glm::vec3 v3;
	int material;
};

struct Light {
	glm::vec3 pos;
	float isDirectional;
	glm::vec3 colour;
	float radius;
	float constant;
	float linear;
	float quadratic;
	//Pre calc maximum distance light is visible from
	float maxDist;
};

struct Material {
	glm::vec3 colour;
	float shininess;
	float reflection;
	float refIndex;
	int opaque;
	float paddingA;
};
