#version 430
#extension GL_ARB_compute_variable_group_size : enable

//The image being written to
uniform writeonly image2D imgOut;

layout(local_size_variable) in;
void main(){
	//Get the position of the current pixel
	ivec2 pixelPos = ivec2(gl_GlobalInvocationID.xy);
	//float r = gl_GlobalInvocationID.x / gl_WorkGroupSize.x;
	//float g = gl_GlobalInvocationID.y / gl_WorkGroupSize.y;	
	imageStore(imgOut, pixelPos, vec4(r,g,1.0,1.0));
}