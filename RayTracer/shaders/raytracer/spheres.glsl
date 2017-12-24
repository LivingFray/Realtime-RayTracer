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
bool getSphereCollision(Sphere s, vec3 rayOrigin, vec3 rayDirection, inout Collision c) {
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
		if(closest >= 0.0 && c.dist>closest){
			c.dist = closest;
			c.hit = true;
			c.hitAt = rayOrigin + c.dist * rayDirection;
			c.hitNorm = normalize(c.hitAt - s.pos);
			c.hitColour = s.colour;
			c.hitShininess = s.shininess;
			return true;
		}
	}
	return false;
}

bool hasSphereCollision(Sphere s, vec3 rayOrigin, vec3 rayDirection) {
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
}