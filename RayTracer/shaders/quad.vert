#version 430

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inTex;

out vec2 texCoords;

void main(){
	texCoords = inTex;
	gl_Position = vec4(inPos, 1.0, 1.0);
}