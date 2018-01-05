uniform int numX;
uniform int numY;
uniform int numZ;
uniform float sizeX;
uniform float sizeY;
uniform float sizeZ;
uniform float gridMinX;
uniform float gridMinY;
uniform float gridMinZ;
uniform float gridMaxX;
uniform float gridMaxY;
uniform float gridMaxZ;

struct Collision {
	float dist;
	vec3 hitAt;
	vec3 hitNorm;
	bool hit;
	int material;
};

//Prevent reflected rays colliding with the object they originated from
#define BIAS 0.001

#define NUM_SHADOW_RAYS 1

#define SKY_COLOR vec3(0.529, 0.808, 0.980)

#define MAX_DEPTH 3

#define MIN_CONTR 0.05

//Returns if the ray hits anything
bool hasCollision(vec3 rayOrigin, vec3 rayDirection, float minDist, float maxDist);

//Returns the closest collision along the ray 
Collision getCollision(vec3 rayOrigin, vec3 rayDirection);

//Gets the pixel colour where the ray hits, with relfection
vec3 getPixelColourReflect(vec3 rayOrigin, vec3 rayDirection);