#version 430
#extension GL_ARB_compute_variable_group_size : enable

//The image being written to
layout(rgba32f, binding = 0) uniform writeonly image2D imgOut;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main(){
	//Get the position of the current pixel
	ivec2 pixelPos = ivec2(gl_GlobalInvocationID.xy);
	uvec3 c = gl_NumWorkGroups * gl_WorkGroupSize;	
	imageStore(imgOut, pixelPos, vec4(float(pixelPos.x) / c.x, float(pixelPos.y) / c.y, 0.0, 1.0));
}