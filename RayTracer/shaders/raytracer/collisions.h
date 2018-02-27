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
	vec3 pos;
	vec3 norm;
	bool hit;
	int material;
#ifdef DRAW_REGGRID
	vec3 dbgColour;
#endif
};

//Prevent reflected rays colliding with the object they originated from
#define BIAS 0.001

#ifndef NUM_SHADOW_RAYS
#define NUM_SHADOW_RAYS 1
#endif

#define SKY_COLOR vec3(0.529, 0.808, 0.980)

#ifndef MAX_DEPTH
#define MAX_DEPTH 3
#endif

#ifndef MIN_CONTR
#define MIN_CONTR 0.005
#endif

#ifndef MAX_REFLECT
#define MAX_REFLECT 4
#endif

#ifndef MAX_REFRACT
#define MAX_REFRACT 2
#endif

//Returns if the ray hits anything
bool hasCollision(vec3 rayOrigin, vec3 rayDirection, float minDist, float maxDist);

//Returns the closest collision along the ray 
Collision getCollision(vec3 rayOrigin, vec3 rayDirection);

//Gets the pixel colour where the ray hits, with relfection
vec3 getPixelColourReflectAndRefract(vec3 rayOrigin, vec3 rayDirection);

float getFresnel(float currentInd, float newInd, vec3 normal, vec3 incident, float reflectivity);