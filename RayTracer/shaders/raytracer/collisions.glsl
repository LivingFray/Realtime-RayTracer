// /*DEBUG*/ => //*DEBUG*/ Toggles a debug line

//Finds the colour at the point the ray hits
vec3 getPixelColour(vec3 rayOrigin, vec3 rayDirection){
	//
	//Initial ray intersections
	//
	Collision c;
	//Start with infinite distance collision
	c.dist = 1.0 / 0.0;
	c.hit = false;
	//Loop through each sphere
	int listStart;
	int listEnd;
	DDA dda;
	
	/*DEBUG*/vec3 debugColour = vec3(0.0, 0.0, 0.0);//*/
	//Track if a collision was made, if so no need to continue traversing grid
	bool hitSphere = false;
	if(initSphereListRay(rayOrigin, rayDirection, dda, listStart, listEnd)){
		while(getNextSphereList(dda, listStart, listEnd) && !hitSphere){
			/*DEBUG*/debugColour += vec3(0.01, 0.01, 0.01);//*/
			for(int i=listStart; i<listEnd; i++){
				hitSphere = getSphereCollision(spheres[sphereLists[i]], rayOrigin, rayDirection, c) || hitSphere;
			}
		}
	}
	//Loop through each plane
	for(int i=0; i<planes.length(); i++){
		getPlaneCollision(planes[i], rayOrigin, rayDirection, c);
	}
	//No collisions just draw the sky
	if(!c.hit){
		/*DEBUG*/return SKY_COLOR + debugColour;/*/*//*
		return SKY_COLOR;//*/
	}
	//
	//Lighting
	//
	vec3 lightColour = vec3(0.0, 0.0, 0.0);//Add ambient here?
	//Loop through each light to calculate lighting
	for(int i=0;i<lights.length(); i++){
		addLighting(lightColour, lights[i], c, rayDirection);
	}
	/*DEBUG*/return lightColour + debugColour;/*/*//*
	return lightColour;//*/
}

bool hasCollision(vec3 rayOrigin, vec3 rayDirection, float minDist, float maxDist){
	//Loop through each plane
	for(int i=0; i<planes.length(); i++){
		if(hasPlaneCollision(planes[i], rayOrigin, rayDirection, minDist, maxDist)) {
			//return true;
		}
	}
	//Loop through each sphere
	int listStart;
	int listEnd;
	DDA dda;
	if(initSphereListRay(rayOrigin, rayDirection, dda, listStart, listEnd)){
		while(getNextSphereList(dda, listStart, listEnd)){
			for(int i=listStart; i<listEnd; i++){
				if(hasSphereCollision(spheres[sphereLists[i]], rayOrigin, rayDirection, minDist, maxDist)) {
					return true;
				}
			}
		}
	}
	return false;
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
