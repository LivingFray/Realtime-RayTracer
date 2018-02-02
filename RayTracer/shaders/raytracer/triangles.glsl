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