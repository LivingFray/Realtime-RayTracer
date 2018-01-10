// /*DEBUG*/ => //*DEBUG*/ Toggles a debug line

bool hasCollision(vec3 rayOrigin, vec3 rayDirection, float minDist, float maxDist){
	//Loop through each plane
	for(int i=0; i<planes.length(); i++){
		if(hasPlaneCollision(planes[i], rayOrigin, rayDirection, minDist, maxDist)) {
			return true;
		}
	}
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
	return false;
}

Collision getCollision(vec3 rayOrigin, vec3 rayDirection) {
	Collision c;
	//Start with infinite distance collision
	c.dist = 1.0 / 0.0;
	c.hit = false;
	//Loop through each sphere
	int listStart;
	int listEnd;
	DDA dda;
	//Track if a collision was made, if so no need to continue traversing grid
	bool hitSphere = false;
	if(initSphereListRay(rayOrigin, rayDirection, dda, listStart, listEnd)){
		while(getNextSphereList(dda, listStart, listEnd) && !hitSphere){
			for(int i=listStart; i<listEnd; i++){
				hitSphere = getSphereCollision(spheres[sphereLists[i]], rayOrigin, rayDirection, c) || hitSphere;
			}
		}
	}
	//Loop through each plane
	for(int i=0; i<planes.length(); i++){
		getPlaneCollision(planes[i], rayOrigin, rayDirection, c);
	}
	return c;
}

vec3 getPixelColourReflect(vec3 rayOrigin, vec3 rayDirection) {
	vec3 pixelColour = vec3(0.0, 0.0, 0.0);
	float amount = 1.0;
	for(int i = 0; i < MAX_DEPTH; i++){
		Collision col = getCollision(rayOrigin, rayDirection);
		if(!col.hit) {
			//Sky
			pixelColour += SKY_COLOR * amount;
			break;
		}
		vec3 lightColour = vec3(0.0, 0.0, 0.0);
		Material m = materials[col.material];
		if(m.opaque) {
			//Loop through each light to calculate lighting
			//TODO: Skip if too reflective
			for(int j=0;j<lights.length(); j++){
				addLighting(lightColour, lights[j], col, rayDirection);
			}
		} else {
			//Refraction
			
		}
		//TODO: Inside -> Outside fresnel
		float r = getFresnel(1.0, m.refIndex, rayDirection, col.hitNorm, m.reflection);
		pixelColour += lightColour * (1.0 - r) * amount;
		amount *= r; //Reduce contribution for next ray
		if (amount < MIN_CONTR) {
			break; //So little contribution not worth persuing
		}
		rayDirection = reflect(rayDirection, col.hitNorm);
		//Prevent self intersection
		rayOrigin = col.hitAt + rayDirection * BIAS;
	}
	return pixelColour;
}

float getFresnel(float currentInd, float newInd, vec3 normal, vec3 incident, float reflectivity) {
	if(reflectivity == 0.0) {
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

/*
Reflections:
Cast primary ray:
If(NumReflections < MAX_DEPTH or Amount_Contributed > MIN_CONTR):
	colour += GetPixelColour(Reflection_Ray)

getPixelColour(Ray):
	for each object:
		Collision = GetCollision(Ray)
	for each light:
		PixelColour += ApplyLighting(Collision)
	return PixelColour
	
getPixelColourReflect(Ray):
	for each object:
		Collision = GetCollision(Ray)
	for each light:
		PixelColour += ApplyLighting(Collision)
	R = CalculateReflection(Collision)
	if(ReflectionContributes(R)):
		PixelColour += getPixelColourReflect(R) * ReflectAmount
	return PixelColour

getPixelColourReflectIter(Ray):
	R = Ray
	do:
		PixelColour += getPixelColour(Ray) * ReflectAmount
		R = CalculateReflection(Collision)
	while(ReflectionContributes(R))
	return PixelColour
	
*/
