#version 430
#extension GL_ARB_compute_variable_group_size : enable
//Increasing this DESTROYS the framerate (Likely a result of less being done in parallel)
#define GROUP_SIZE 1
layout(local_size_x = GROUP_SIZE, local_size_y = GROUP_SIZE, local_size_z = 1) in;

//Debug, shows the regular grid spheres are held in
//#define DRAW_REGGRID

//#define DEBUG_RELFECT

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

struct Triangle {
	vec3 v1;
	float paddingA;
	vec3 v2;
	float paddingB;
	vec3 v3;
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

//TODO: Acceleration structure
layout(std140, binding = 7) buffer TriangleBuffer {
	Triangle triangles[];
};

//Camera variables
uniform float twiceTanFovY;
uniform float cameraWidth;
uniform float cameraHeight;
uniform mat4 cameraMatrix; 
// 
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
//Adds lighting to the pixel
void addLighting(inout vec3 lightColour, Light l, Collision c, vec3 rayDirection);
//Applies lighting to the pixel at 1/fraction the normal brightness
void applyLighting(inout vec3 lightColour, vec3 lightDir, vec3 norm, vec3 rayDirection, Light l, float hitShininess, vec3 hitColour, float dist, float fraction); 
//Returns whether there was a collision and updates Collision struct accordingly
bool getPlaneCollision(Plane p, vec3 rayOrigin, vec3 rayDirection, inout Collision c);
//Returns whether the plane intersects with the ray
bool hasPlaneCollision(Plane p, vec3 rayOrigin, vec3 rayDirection, float minDist, float maxDist); 
//DDA variables
struct DDA {
	int stepX;
	int stepY;
	int stepZ;
	float deltaX;
	float deltaY;
	float deltaZ;
	int gridX;
	int gridY;
	int gridZ;
	float maxX;
	float maxY;
	float maxZ;
	bool firstSphereIt;
	//Used to help collision code know when it can stop early
	float distToEdge;
};

//Gets the next grid that contains a possible collision
bool getNextSphereList(inout DDA dda, inout int listStart, inout int listEnd);

//Resets the grid traversal algorithm
bool initSphereListRay(vec3 rayOrigin, vec3 rayDirection, inout DDA dda, inout int listStart, inout int listEnd);

//Returns if a ray intersects with the grid, setting the distance in dist if it does
bool distToAABB(vec3 rayOrigin, vec3 rayDirection, inout float dist); 
//Returns whether there was a collision and updates Collision struct accordingly
bool getSphereCollision(Sphere s, vec3 rayOrigin, vec3 rayDirection, inout Collision col);
//Returns whether there exists a collision between the ray and the sphere
bool hasSphereCollision(Sphere s, vec3 rayOrigin, vec3 rayDirection, float minDist, float maxDist); 
//Returns whether there was a collision and updates Collision struct accordingly
bool getTriangleCollision(Triangle t, vec3 rayOrigin, vec3 rayDirection, inout Collision col);
//Returns whether there exists a collision between the ray and the triangle
bool hasTriangleCollision(Triangle t, vec3 rayOrigin, vec3 rayDirection, float minDist, float maxDist); 
// 
// /*DEBUG*/ => //*DEBUG*/ Toggles a debug line

bool hasCollision(vec3 rayOrigin, vec3 rayDirection, float minDist, float maxDist){
	//Loop through each plane
	for(int i=0; i<planes.length(); i++){
		if(hasPlaneCollision(planes[i], rayOrigin, rayDirection, minDist, maxDist)) {
			return true;
		}
	}
#ifdef DONT_USE_GRID
	for(int i = 0; i < spheres.length(); i++){
		if(hasSphereCollision(spheres[i], rayOrigin, rayDirection, minDist, maxDist)) {
			return true;
		}
	}
#else
	//Loop through each sphere
	int listStart;
	int listEnd;
	DDA dda;
	if(initSphereListRay(rayOrigin, rayDirection, dda, listStart, listEnd)){
		while(getNextSphereList(dda, listStart, listEnd)){
			for(int i = listStart; i < listEnd; i++){
				if(hasSphereCollision(spheres[sphereLists[i]], rayOrigin, rayDirection, minDist, maxDist)) {
					return true;
				}
			}
		}
	}
#endif
	//Loop through each triangle
		for(int i=0; i<triangles.length(); i++){
		if(hasTriangleCollision(triangles[i], rayOrigin, rayDirection, minDist, maxDist)) {
			return true;
		}
	}
	return false;
}

Collision getCollision(vec3 rayOrigin, vec3 rayDirection) {
	Collision c;
#ifdef DRAW_REGGRID
	c.dbgColour = vec3(0.0, 0.0, 0.0);
#endif
	//Start with infinite distance collision
	c.dist = 1.0 / 0.0;
	c.hit = false;
#ifdef DONT_USE_GRID
	for(int i=0; i<spheres.length(); i++) {
		getSphereCollision(spheres[i], rayOrigin, rayDirection, c);
	}
#else
	//Loop through each sphere
	int listStart;
	int listEnd;
	DDA dda;
	//Track if a collision was made, if so no need to continue traversing grid
	bool hitSphere = false;
	if(initSphereListRay(rayOrigin, rayDirection, dda, listStart, listEnd)){
		while(getNextSphereList(dda, listStart, listEnd) && !hitSphere){
#ifdef DRAW_REGGRID
			c.dbgColour += vec3(0.1, 0.1, 0.1);
#endif
			for(int i=listStart; i<listEnd; i++){
				hitSphere = (getSphereCollision(spheres[sphereLists[i]], rayOrigin, rayDirection, c) && c.dist < dda.distToEdge) || hitSphere;
#ifndef EARLY_GRID_EXIT
				hitSphere = false;
#endif
			}
		}
	}
#endif
	//Loop through each plane
	for(int i=0; i<planes.length(); i++){
		getPlaneCollision(planes[i], rayOrigin, rayDirection, c);
	}
	//Loop through each triangle
	for(int i=0; i<triangles.length(); i++){
		getTriangleCollision(triangles[i], rayOrigin, rayDirection, c);
	}
	return c;
}

vec3 getPixelColourReflectAndRefract(vec3 rayOrigin, vec3 rayDirection) {
	vec3 pixelColour = vec3(0.0, 0.0, 0.0);
	struct AdditionalRay {
		vec3 rayOrigin;
		vec3 rayDirection;
		float contr;
	};
	struct Iteration {
		AdditionalRay rays[2];
		int numRays;
	};
	Iteration iterations[MAX_DEPTH];
	int numIterations = 1;
	int numReflect = 0;
	int numRefract = 0;
	AdditionalRay primaryRay;
	primaryRay.rayOrigin = rayOrigin;
	primaryRay.rayDirection = rayDirection;
	primaryRay.contr = 1.0;
	Iteration firstIter;
	firstIter.rays[0] = primaryRay;
	firstIter.numRays = 1;
	iterations[0] = firstIter;
	int iterExplored = 0;
	while (numIterations > 0 && iterExplored < MAX_DEPTH) {
		//Pop ray off stack
		numIterations--;
		Iteration it = iterations[numIterations];
		//For each ray in current iteration, explore rays and add new rays to list
		for(int i = 0; i < it.numRays; i++) {
			iterExplored++;
			AdditionalRay ray = it.rays[i];
			Iteration nextIter;
			nextIter.numRays = 0;
			//Find collision
			Collision col = getCollision(ray.rayOrigin, ray.rayDirection);
			//If nothing hit, add the sky
			if (!col.hit) {
				pixelColour += SKY_COLOR * ray.contr;
				continue;
			}
			//Get material of collided object
			Material mat = materials[col.material];
			float startRef = 1.0;
			float endRef = mat.refIndex;
			//Flip normal if hitting back of surface
			if(dot(col.norm, rayDirection) > 0) {
				col.norm *= -1;
				startRef = mat.refIndex;
				endRef = 1.0;
			}
			//Get the proportion of light that is reflected
#ifndef IGNORE_FRESNEL
			float reflectAmount = getFresnel(startRef, endRef, ray.rayDirection, col.norm, mat.reflection);
#else
			float reflectAmount = mat.reflection;
#endif
			float transmitAmount = 1.0 - reflectAmount;

			//If object is solid apply phong lighting
			if (mat.opaque != 0) {
#ifdef DONT_DRAW_LIGHTS
				vec3 lightColour = mat.colour;
#else
				vec3 lightColour = vec3(0.0, 0.0, 0.0);
				for(int j=0;j<lights.length(); j++){
					addLighting(lightColour, lights[j], col, ray.rayDirection);
				}
#endif
#ifdef DRAW_REGGRID
				pixelColour += lightColour * transmitAmount * ray.contr + col.dbgColour;
#else
				pixelColour += lightColour * transmitAmount * ray.contr;
#endif
			} else if (ray.contr * transmitAmount > MIN_CONTR && numRefract < MAX_REFRACT) {
#ifdef DEBUG_REFRACT
				pixelColour += vec3(0.1, 0.0, 0.0);
#endif
				numRefract++;
				//Apply refraction
				AdditionalRay refractRay;
				//Check arguments are correct way round
				refractRay.rayDirection = refract(ray.rayDirection, col.norm, startRef / endRef);
				refractRay.rayOrigin = col.pos + refractRay.rayDirection * BIAS;
				refractRay.contr = ray.contr * transmitAmount;
				//refractRay.refIndex = mat.refIndex;
				//Push new ray onto stack
				nextIter.rays[nextIter.numRays] = refractRay;
				nextIter.numRays++;
				//TODO: Apply Beer's law here
			}
			if (ray.contr * reflectAmount > MIN_CONTR && numReflect < MAX_REFLECT) {
#ifdef DEBUG_REFLECT
				pixelColour += vec3(0.0, 0.0, 0.1);
#endif
				numReflect++;
				//Apply Reflection
				AdditionalRay reflectRay;
				reflectRay.rayDirection = reflect(ray.rayDirection, col.norm);
				reflectRay.rayOrigin = col.pos + reflectRay.rayDirection * BIAS;
				reflectRay.contr = ray.contr * reflectAmount;
				//reflectRay.refIndex = ray.refIndex;
				//Push new ray onto stack
				nextIter.rays[nextIter.numRays] = reflectRay;
				nextIter.numRays++;
			}
			//Add next iteration to list
			if(numIterations < MAX_DEPTH && nextIter.numRays > 0) {
				iterations[numIterations] = nextIter;
				numIterations++;
			}
		}
	}
	return pixelColour;
}

float getFresnel(float currentInd, float newInd, vec3 normal, vec3 incident, float reflectivity) {
	if (reflectivity == 0.0) {
		return 0.0;
	}
	float r0 = (currentInd - newInd) / (currentInd + newInd);
	r0 *= r0;
	float cosX = -dot(normal, incident);
	if (currentInd > newInd) {
		float ratio = currentInd / newInd;
		float sinT2 = ratio * ratio * (1.0 - cosX * cosX);
		if (sinT2 > 1.0) {
			return 1.0;
		}
		cosX = sqrt(1.0 - sinT2);
	}
	float x = 1.0 - cosX;
	
	return reflectivity + (1.0 - reflectivity) * (r0 + (1.0 - r0) * x * x * x * x * x);
}
 

void addLighting(inout vec3 lightColour, Light l, Collision c, vec3 rayDirection){
	vec3 lightDir;
	if(l.isDirectional>0.0){
		lightDir = -l.pos;
	} else {
		lightDir = l.pos - c.pos;
	}
	float dist = length(lightDir);
	lightDir /= dist; //Normalize
	//For efficiency don't calculate effect of distant light sources
	//Also check for shadows here
	Material m = materials[c.material];
	if((l.isDirectional>0.0 || dist < l.maxDist)) {
#if (NUM_SHADOW_RAYS <= 0)
		applyLighting(lightColour, lightDir, c.norm, rayDirection, l, m.shininess, m.colour, dist, 1.0);
#endif
#if ((NUM_SHADOW_RAYS > 0) && (NUM_SHADOW_RAYS % 2 == 1))
		float frac;
		float maxDist;
		if(l.isDirectional>0.0) {
			frac = 1.0;
			maxDist = 1.0 / 0.0;
		} else {
			frac = NUM_SHADOW_RAYS * NUM_SHADOW_RAYS;
			maxDist = dist;
		}
		if(!hasCollision(c.pos, lightDir, BIAS, maxDist)){
			applyLighting(lightColour, lightDir, c.norm, rayDirection, l, m.shininess, m.colour, dist, frac);
		}
#else
#if (NUM_SHADOW_RAYS > 0)
		float frac;
		float maxDist;
		if(l.isDirectional>0.0) {
			frac = 1.0;
			maxDist = 1.0 / 0.0;
			if(!hasCollision(c.pos, lightDir, BIAS, maxDist)){
				applyLighting(lightColour, lightDir, c.norm, rayDirection, l, m.shininess, m.colour, dist, frac);
			}
		}
#endif
#endif
/*
Range from -LR to +LR
Fire ray in centre
For NUM_SHADOW_RAYS/2 Fire ray in neg from centre at interval of LR / NUM_SHADOW_RAYS/2 
*/
#if (NUM_SHADOW_RAYS > 1)
		if(l.isDirectional <= 0.0){
			vec3 lightUp = vec3(0.0, 1.0, 0.0);
			vec3 lightRight = cross(lightDir, lightUp);
			lightUp = cross(lightRight, lightDir);
			int halfNumRays = NUM_SHADOW_RAYS/2;
			lightRight *= l.radius / halfNumRays;
			lightUp *= l.radius / halfNumRays;
			for(int x = 1; x <= halfNumRays; x++) {
				for(int y = 1; y <= halfNumRays; y++) {
					vec3 newDir = (l.pos + lightRight * x + lightUp * y) - c.pos;
					dist = length(newDir);
					newDir /= dist;
					if(!hasCollision(c.pos, newDir, BIAS, dist)){
						applyLighting(lightColour, newDir, c.norm, rayDirection, l, m.shininess, m.colour, dist, NUM_SHADOW_RAYS * NUM_SHADOW_RAYS);
					}
					newDir = (l.pos - lightRight * x + lightUp * y) - c.pos;
					dist = length(newDir);
					newDir /= dist;
					if(!hasCollision(c.pos, newDir, BIAS, dist)){
						applyLighting(lightColour, newDir, c.norm, rayDirection, l, m.shininess, m.colour, dist, NUM_SHADOW_RAYS * NUM_SHADOW_RAYS);
					}
					newDir = (l.pos + lightRight * x - lightUp * y) - c.pos;
					dist = length(newDir);
					newDir /= dist;
					if(!hasCollision(c.pos, newDir, BIAS, dist)){
						applyLighting(lightColour, newDir, c.norm, rayDirection, l, m.shininess, m.colour, dist, NUM_SHADOW_RAYS * NUM_SHADOW_RAYS);
					}
					newDir = (l.pos - lightRight * x - lightUp * y) - c.pos;
					dist = length(newDir);
					newDir /= dist;
					if(!hasCollision(c.pos, newDir, BIAS, dist)){
						applyLighting(lightColour, newDir, c.norm, rayDirection, l, m.shininess, m.colour, dist, NUM_SHADOW_RAYS * NUM_SHADOW_RAYS);
					}
				}
			}
		}
#endif
	}
}

void applyLighting(inout vec3 lightColour, vec3 lightDir, vec3 norm, vec3 rayDirection, Light l, float hitShininess, vec3 hitColour, float dist, float fraction){
	float diff = max(0.0, dot(norm, lightDir));
	vec3 halfwayDir = normalize(lightDir - rayDirection);
	float spec = pow(max(dot(norm, halfwayDir), 0.0), hitShininess);
	float att = 1.0 / (l.constant + l.linear * dist + 
		l.quadratic * (dist * dist));
	//Directional light does not dim with distance
	if(l.isDirectional > 0.0) {
		att = 1.0;
	}
	lightColour += (hitColour * diff + spec) * l.colour * att / fraction;
} 
void main(){
	//Get the position of the current pixel
	ivec2 pixelPos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 screenSize = imageSize(imgOut);
	//Calculate ray
	//Use view matrix to set origin
	vec3 rayOrigin = vec3(cameraMatrix * vec4(0.0, 0.0, 0.0, 1.0));
	//Convert pixel position to world space
	float x = cameraWidth * (pixelPos.x + 0.5) / screenSize.x - cameraWidth * 0.5;
	float y = cameraHeight * (pixelPos.y + 0.5) / screenSize.y - cameraHeight * 0.5;
	float z = cameraHeight / twiceTanFovY;
	//Apply view matrix to direction
	vec3 rayDirection = normalize(mat3(cameraMatrix) * vec3(x, y, z));
	//Fire ray
	vec3 pixelColour = getPixelColourReflectAndRefract(rayOrigin, rayDirection);//getPixelColour(rayOrigin, rayDirection);
	imageStore(imgOut, pixelPos, vec4(pixelColour, 1.0));
} 
// P = rO + t(rD)
// 0 = N . (P - p0)
// 0 = N . (p0 - rO + t(rD))
// t = (p0 - rO).N / (rD.N)
// If rD.N = 0: Parallel (For this treat as no intersection)
// If t < 0: Behind ray origin
bool getPlaneCollision(Plane p, vec3 rayOrigin, vec3 rayDirection, inout Collision c){
	float rDN = dot(rayDirection, p.norm);
	//Check not zero (or very close to)
	if(abs(rDN)>0.0001 && dot(-rayDirection, p.norm) > 0.0) {
		float t = dot((p.pos - rayOrigin), p.norm) / rDN;
		if(t > 0.0 && t < c.dist){
			c.hit = true;
			c.dist = t;
			c.pos = rayOrigin + c.dist * rayDirection;
			c.norm = p.norm;
			c.material = p.material;
			return true;
		}
	}
	return false;
}

bool hasPlaneCollision(Plane p, vec3 rayOrigin, vec3 rayDirection, float minDist, float maxDist) {
	float rDN = dot(rayDirection, p.norm);
	//Check not zero (or very close to)
	if(abs(rDN)>0.0001){
		float t = dot((p.pos - rayOrigin), p.norm) / rDN;
		if(t > minDist && t <= maxDist){
			return true;
		}
	}
	return false;
} 
bool distToAABB(vec3 rayOrigin, vec3 rayDirection, inout float dist) {
	vec3 dirInv = vec3(1.0 / rayDirection.x, 1.0 / rayDirection.y, 1.0 / rayDirection.z);
    float tx1 = (gridMinX - rayOrigin.x)*dirInv.x;
    float tx2 = (gridMaxX - rayOrigin.x)*dirInv.x;
 
    float tmin = min(tx1, tx2);
    float tmax = max(tx1, tx2);
 
    float ty1 = (gridMinY - rayOrigin.y)*dirInv.y;
    float ty2 = (gridMaxY - rayOrigin.y)*dirInv.y;
 
    tmin = max(tmin, min(ty1, ty2));
    tmax = min(tmax, max(ty1, ty2));
	
	float tz1 = (gridMinZ - rayOrigin.z)*dirInv.z;
    float tz2 = (gridMaxZ - rayOrigin.z)*dirInv.z;
 
    tmin = max(tmin, min(tz1, tz2));
    tmax = min(tmax, max(tz1, tz2));
	
	dist = tmin;
    return tmax >= tmin && tmax > 0.0;
}

bool initSphereListRay(vec3 rayOrigin, vec3 rayDirection, inout DDA dda, inout int listStart, inout int listEnd) {
	listStart = 0;
	listEnd = 0;
	dda.stepX = rayDirection.x > 0 ? 1 : -1;
	dda.stepY = rayDirection.y > 0 ? 1 : -1;
	dda.stepZ = rayDirection.z > 0 ? 1 : -1;

	//Keep ray in bounds
	float dist;
	if(distToAABB(rayOrigin, rayDirection, dist)){
		rayOrigin = rayOrigin + dist * rayDirection;
	} else {
		return false;
	}
	
	dda.deltaX = sizeX / rayDirection.x;
	dda.deltaY = sizeY / rayDirection.y;
	dda.deltaZ = sizeZ / rayDirection.z;
	
	//Make negative directions positive
	dda.deltaX *= dda.stepX;
	dda.deltaY *= dda.stepY;
	dda.deltaZ *= dda.stepZ;
	
	dda.gridX = clamp(int((rayOrigin.x - gridMinX) / sizeX), 0, numX - 1);
	dda.gridY = clamp(int((rayOrigin.y - gridMinY) / sizeY), 0, numY - 1);
	dda.gridZ = clamp(int((rayOrigin.z - gridMinZ) / sizeZ), 0, numZ - 1);
	dda.maxX = ((dda.gridX + max(0, dda.stepX))*sizeX + gridMinX - rayOrigin.x) / rayDirection.x;
	dda.maxY = ((dda.gridY + max(0, dda.stepY))*sizeY + gridMinY - rayOrigin.y) / rayDirection.y;
	dda.maxZ = ((dda.gridZ + max(0, dda.stepZ))*sizeZ + gridMinZ - rayOrigin.z) / rayDirection.z;
	dda.firstSphereIt = true;
	if(dda.maxX < dda.maxY) {
		if(dda.maxX < dda.maxZ) {
			dda.distToEdge = dda.maxX;
		} else {
			dda.distToEdge = dda.maxZ;
		}
	} else {
		if(dda.maxY < dda.maxZ) {
			dda.distToEdge = dda.maxY;
		} else {
			dda.distToEdge = dda.maxZ;
		}
	}
	return true;
}
/*
Store index of last sphere
Get first index from previous
If no previous first is 0

LIST_START = Index of first relevant sphere in sphereList
LIST_END = Index of first non relevant sphere in sphereList

If no matches LIST_START = LIST_END

*/
bool getNextSphereList(inout DDA dda, inout int listStart, inout int listEnd) {
	int index;
	if(!dda.firstSphereIt){
		if(dda.maxX < dda.maxY) {
			if(dda.maxX < dda.maxZ) {
				dda.gridX = dda.gridX + dda.stepX;
				if(dda.gridX == numX || dda.gridX == -1)
					return false; 
				dda.maxX = dda.maxX + dda.deltaX;
				dda.distToEdge += dda.deltaX;
			} else  {
				dda.gridZ = dda.gridZ + dda.stepZ;
				if(dda.gridZ == numZ || dda.gridZ == -1)
					return false;
				dda.maxZ = dda.maxZ + dda.deltaZ;
				dda.distToEdge += dda.deltaZ;
			}
		} else  {
			if(dda.maxY < dda.maxZ) {
				dda.gridY = dda.gridY + dda.stepY;
				if(dda.gridY == numY || dda.gridY == -1)
					return false;
				dda.maxY = dda.maxY + dda.deltaY;
				dda.distToEdge += dda.deltaY;
			} else  {
				dda.gridZ = dda.gridZ + dda.stepZ;
				if(dda.gridZ == numZ || dda.gridZ == -1)
					return false;
				dda.maxZ = dda.maxZ + dda.deltaZ;
				dda.distToEdge += dda.deltaZ;
			}
		}
	}
	dda.firstSphereIt = false;
	index = dda.gridX + dda.gridY * numX + dda.gridZ * numX * numY;
	listStart = min(index, 1) * sphereGrid[index - 1];
	listEnd = sphereGrid[index];
	return true;
} 
// P = rO + t(rD) //Ray equation
// r = |P - C|    //Sphere equation
////After much rearranging we get:
// t = -b +/- sqrt(b*b - c)
// Where:
// b = (rO - C) . rD
// c = (rO - C).(rO - C) - r*r
// If b * b - c < 0: No Solutions
// If b * b - c = 0: 1 Solution
// If b * b - c > 0: 2 Solutions
bool getSphereCollision(Sphere s, vec3 rayOrigin, vec3 rayDirection, inout Collision col) {
	vec3 rOC = rayOrigin - s.pos;
	float b = dot(rOC, rayDirection);
	float c = dot(rOC, rOC) - s.radius * s.radius;
	float disc = b * b - c;
	//Check for solution
	if(disc >= 0.0){
		float rt = sqrt(disc);
		float first = -b + rt;
		float second = -b - rt;
		float closest = min(first, second);
		//Closest is behind camera, try other one
		if(closest < 0.0){
			closest = max(first, second);
		}
		if(closest >= 0.0 && col.dist>closest){
			col.dist = closest;
			col.hit = true;
			col.pos = rayOrigin + col.dist * rayDirection;
			vec3 n = col.pos - s.pos;
			//Flip normal if inside sphere
			//n *= length(n) > length(rayOrigin - s.pos) ? -1.0 : 1.0;
			col.norm = normalize(n);
			
			col.material = s.material;
			return true;
		}
	}
	return false;
}

bool hasSphereCollision(Sphere s, vec3 rayOrigin, vec3 rayDirection, float minDist, float maxDist) {
	vec3 rOC = rayOrigin - s.pos;
	float b = dot(rOC, rayDirection);
	float c = dot(rOC, rOC) - s.radius * s.radius;
	//Check for solution
	float disc = b * b - c;
	//Check for solution
	if(disc >= 0.0){
		float rt = sqrt(disc);
		float first = -b + rt;
		if(first >= minDist && (first <= maxDist || maxDist < 0)){
			return true;
		}
	}
	return false;
} 
bool getTriangleCollision(Triangle t, vec3 rayOrigin, vec3 rayDirection, inout Collision col) {
	vec3 edgeA = t.v2 - t.v1;
	vec3 edgeB = t.v3 - t.v1;
	//TODO DETERMINE WHAT H IS AND NAME SENSIBLY
	vec3 h = cross(rayDirection, edgeB);
	float a = dot(edgeA, h);
	//Prevent divide by 0 errors
	if (abs(a) < BIAS) {
		return false;
	}
	float f = 1.0 / a;
	vec3 s = rayOrigin - t.v1;
	float u = f * dot(s, h);
	if (u < 0.0 || u > 1.0) {
		return false;
	}
	vec3 q = cross(s, edgeA);
	float v = f * dot(rayDirection, q);
	if (v < 0.0 || u + v > 1.0) {
		return false;
	}
	// At this stage we can compute t to find out where the intersection point is on the line.
	float dist = f * dot(edgeB, q);
	// If t is positive, collided
	if (dist > 0.0 && col.dist>dist) {
		col.dist = dist;
		col.hit = true;
		col.pos = rayOrigin + col.dist * rayDirection;
		col.norm = normalize(cross(edgeA, edgeB));
		col.material = t.material;
		return true;
	}
	return false;
}

//Returns whether there exists a collision between the ray and the triangle
bool hasTriangleCollision(Triangle t, vec3 rayOrigin, vec3 rayDirection, float minDist, float maxDist) {
	vec3 edgeA = t.v2 - t.v1;
	vec3 edgeB = t.v3 - t.v1;
	//TODO DETERMINE WHAT H IS AND NAME SENSIBLY
	vec3 h = cross(rayDirection, edgeB);
	float a = dot(edgeA, h);
	//Prevent divide by 0 errors
	if (abs(a) < BIAS) {
		return false;
	}
	float f = 1.0 / a;
	vec3 s = rayOrigin - t.v1;
	float u = f * dot(s, h);
	if (u < 0.0 || u > 1.0) {
		return false;
	}
	vec3 q = cross(s, edgeA);
	float v = f * dot(rayDirection, q);
	if (v < 0.0 || u + v > 1.0) {
		return false;
	}
	// At this stage we can compute t to find out where the intersection point is on the line.
	float dist = f * dot(edgeB, q);
	// If t is positive, collided
	return dist > minDist && dist < maxDist;
} 
