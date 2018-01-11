#version 430
#extension GL_ARB_compute_variable_group_size : enable
//Increasing this DESTROYS the framerate (Likely a result of less being done in parallel)
#define GROUP_SIZE 1
layout(local_size_x = GROUP_SIZE, local_size_y = GROUP_SIZE, local_size_z = 1) in;

//Structures must align to a multiple of 4 floats

struct Sphere {
	vec3 pos;
	float radius;
	int material;
	float paddingA, paddingB, paddingC;
};

struct Plane {
	vec3 pos;
	float paddingA;
	vec3 norm;
	int material;
};

//Fun fact, not padding vec3s and putting it at the end breaks everything

struct Light {
	vec3 pos;
	float isDirectional;
	vec3 colour;
	float radius;
	float constant;
	float linear;
	float quadratic;
	//Pre calc maximum distance light is visible from
	float maxDist;
};

//TODO: Beer's law, textures, other things
struct Material {
	vec3 colour;
	float shininess;
	float reflection;
	float refIndex;
	int opaque;
	float paddingA;
};

//The image being written to
layout(rgba32f, binding = 0) uniform writeonly image2D imgOut;
/*
Spheres: A list of every sphere in the scene
SphereGrid: A list of the start and end positions of each grids list of contained sphers
SphereList: A concatanation of lists of spheres contained within the grids
*/
layout(std140, binding = 1) buffer SphereBuffer {
	Sphere spheres[];
};
//WHAT IS WRONG WITH YOU OPENGL, WHY DOES SETTING THIS TO std140 MAKE YOU IGNORE 3/4 OF THE DATA
layout(std430, binding = 2) buffer GridBuffer {
	int sphereGrid[];
};

layout(std430, binding = 3) buffer ListBuffer {
	int sphereLists[];
};

layout(std140, binding = 4) buffer PlaneBuffer {
	Plane planes[];
};

layout(std140, binding = 5) buffer LightBuffer {
	Light lights[];
};

layout(std140, binding = 6) buffer MaterialBuffer {
	Material materials[];
};

//Camera variables
uniform float twiceTanFovY;
uniform float cameraWidth;
uniform float cameraHeight;
uniform mat4 cameraMatrix;