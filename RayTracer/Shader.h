#pragma once
/*
A wrapper for the GLSL shaders
*/
#include <string>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
class Shader {
public:
	Shader();
	Shader(std::string vert, std::string frag);
	Shader(std::string comp);
	Shader(std::string comp, std::vector<std::string> args);
	~Shader();
	GLuint getProgram() { return id; };
	void deleteShader();
private:
	std::string readFile(std::string filename);
	GLuint id;
};

