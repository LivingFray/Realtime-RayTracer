#version 430

in vec2 texCoords;

out vec4 colour;

uniform sampler2D imgOut;

void main(){
	colour = texture(imgOut, texCoords);
}