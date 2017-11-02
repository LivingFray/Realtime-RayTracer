#version 430
#extension GL_ARB_compute_variable_group_size : enable
//Increasing this DESTROYS the framerate (Likely a result of less being done in parallel)
#define GROUP_SIZE 1
layout(local_size_x = GROUP_SIZE, local_size_y = GROUP_SIZE, local_size_z = 1) in;

struct Sphere {
	vec3 pos;
	float radius;
	vec3 colour;
	float padding;
};

//The image being written to
layout(rgba32f, binding = 0) uniform writeonly image2D imgOut;

layout(std140, binding = 4) buffer SphereBuffer {
	Sphere spheres[];
};

uniform float twiceTanFovY;
uniform float cameraWidth;
uniform float cameraHeight;
uniform mat4 cameraMatrix;

vec3 getCollision(vec3 rayOrigin, vec3 rayDirection);

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
	vec3 pixelColour = getCollision(rayOrigin, rayDirection);
	imageStore(imgOut, pixelPos, vec4(pixelColour, 1.0));
}

//Finds the colour at the point the ray hits
vec3 getCollision(vec3 rayOrigin, vec3 rayDirection){
	float d = -1;
	vec3 colour = vec3(0.0, 0.0, 0.0);
	//Loop through each sphere
	for(int i=0; i<spheres.length(); i++){
		Sphere s = spheres[i];
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
			if(closest >= 0.0 && (d<0.0 || d>closest)){
				d = closest;
				colour = s.colour;
			}
		}
	}
	return colour;
}