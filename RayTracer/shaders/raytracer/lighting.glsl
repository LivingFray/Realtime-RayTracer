
void addLighting(inout vec3 lightColour, Light l, Collision c, vec3 rayDirection){
	vec3 lightDir;
	if(l.isDirectional>0.0){
		lightDir = -l.pos;
	} else {
		lightDir = l.pos - c.hitAt;
	}
	float dist = length(lightDir);
	lightDir /= dist; //Normalize
	//For efficiency don't calculate effect of distant light sources
	//Also check for shadows here
	if((l.isDirectional>0.0 || dist < l.maxDist)) {
#if (NUM_SHADOW_RAYS <= 0)
		applyLighting(lightColour, lightDir, c.hitNorm, rayDirection, l, c.hitShininess, c.hitColour, dist, 1.0);
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
		if(!hasCollision(c.hitAt, lightDir, BIAS, maxDist)){
			applyLighting(lightColour, lightDir, c.hitNorm, rayDirection, l, c.hitShininess, c.hitColour, dist, frac);
		}
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
					vec3 newDir = (l.pos + lightRight * x + lightUp * y) - c.hitAt;
					dist = length(newDir);
					newDir /= dist;
					if(!hasCollision(c.hitAt, newDir, BIAS, dist)){
						applyLighting(lightColour, newDir, c.hitNorm, rayDirection, l, c.hitShininess, c.hitColour, dist, NUM_SHADOW_RAYS * NUM_SHADOW_RAYS);
					}
					newDir = (l.pos - lightRight * x + lightUp * y) - c.hitAt;
					dist = length(newDir);
					newDir /= dist;
					if(!hasCollision(c.hitAt, newDir, BIAS, dist)){
						applyLighting(lightColour, newDir, c.hitNorm, rayDirection, l, c.hitShininess, c.hitColour, dist, NUM_SHADOW_RAYS * NUM_SHADOW_RAYS);
					}
					newDir = (l.pos + lightRight * x - lightUp * y) - c.hitAt;
					dist = length(newDir);
					newDir /= dist;
					if(!hasCollision(c.hitAt, newDir, BIAS, dist)){
						applyLighting(lightColour, newDir, c.hitNorm, rayDirection, l, c.hitShininess, c.hitColour, dist, NUM_SHADOW_RAYS * NUM_SHADOW_RAYS);
					}
					newDir = (l.pos - lightRight * x - lightUp * y) - c.hitAt;
					dist = length(newDir);
					newDir /= dist;
					if(!hasCollision(c.hitAt, newDir, BIAS, dist)){
						applyLighting(lightColour, newDir, c.hitNorm, rayDirection, l, c.hitShininess, c.hitColour, dist, NUM_SHADOW_RAYS * NUM_SHADOW_RAYS);
					}
				}
			}
		}
#endif
	}
}

void applyLighting(inout vec3 lightColour, vec3 lightDir, vec3 hitNorm, vec3 rayDirection, Light l, float hitShininess, vec3 hitColour, float dist, float fraction){
	float diff = max(0.0, dot(hitNorm, lightDir));
	vec3 halfwayDir = normalize(lightDir - rayDirection);
	float spec = pow(max(dot(hitNorm, halfwayDir), 0.0), hitShininess);
	float att = 1.0 / (l.constant + l.linear * dist + 
		l.quadratic * (dist * dist));
	//Directional light does not dim with distance
	if(l.isDirectional > 0.0) {
		att = 1.0;
	}
	lightColour += (hitColour * diff + spec) * l.colour * att / fraction;
}