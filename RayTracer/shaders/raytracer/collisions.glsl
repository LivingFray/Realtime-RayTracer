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
		//Loop through each light to calculate lighting
		//TODO: Skip if too reflective
		for(int j=0;j<lights.length(); j++){
			addLighting(lightColour, lights[j], col, rayDirection);
		}
		Material m = materials[col.material];
		pixelColour += lightColour * (1.0 - m.reflection) * amount;
		amount *= m.reflection; //Reduce contribution for next ray
		if (amount < MIN_CONTR) {
			break; //So little contribution not worth persuing
		}
		rayDirection = reflect(rayDirection, col.hitNorm);
		//Prevent self intersection
		rayOrigin = col.hitAt + rayDirection * BIAS;
	}
	return pixelColour;
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
