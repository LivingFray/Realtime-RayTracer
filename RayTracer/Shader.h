#pragma once
/*
A wrapper for the GLSL shaders
*/
#include <string>
#include <map>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
class Shader {
public:
	Shader();
	Shader(std::string vert, std::string frag);
	Shader(std::string comp);
	~Shader();
	GLuint getProgram() { return id; };
	void deleteShader();
private:
	std::string readFile(std::string filename);
	GLuint id;
	std::string key;
	static std::map<std::string, GLuint> shaders;
};

