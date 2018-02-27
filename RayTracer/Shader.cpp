#include "Shader.h"
#include <iostream>
#include <fstream>
#include <vector>

Shader::Shader() {
	id = -1;
}


Shader::Shader(std::string vert, std::string frag) {
	std::string vertString = readFile(vert);
	std::string fragString = readFile(frag);
	const char* vertChars = vertString.c_str();
	const char* fragChars = fragString.c_str();
	if (*vertChars && *fragChars) {
		GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
		GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
		//Compile vertex shader
		glShaderSource(vertex, 1, &vertChars, NULL);
		glCompileShader(vertex);
		//Check for errors
		GLint result = GL_FALSE;
		int logLength;
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &result);
		glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &logLength);
		if (result != GL_TRUE || logLength > 0) {
			std::vector<char> errorMessage(logLength + 1);
			glGetShaderInfoLog(vertex, logLength, NULL, &errorMessage[0]);
			printf("ERROR: %s\n", &errorMessage[0]);
		}
		//Compile fragment shader
		glShaderSource(fragment, 1, &fragChars, NULL);
		glCompileShader(fragment);
		//Check for errors
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &result);
		glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &logLength);
		if (result != GL_TRUE || logLength > 0) {
			std::vector<char> errorMessage(logLength + 1);
			glGetShaderInfoLog(fragment, logLength, NULL, &errorMessage[0]);
			printf("ERROR: %s\n", &errorMessage[0]);
		}
		id = glCreateProgram();
		glAttachShader(id, vertex);
		glAttachShader(id, fragment);
		glLinkProgram(id);
		glGetProgramiv(id, GL_LINK_STATUS, &result);
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logLength);
		if (result != GL_TRUE || logLength > 0) {
			std::vector<char> errorMessage(logLength + 1);
			glGetProgramInfoLog(id, logLength, NULL, &errorMessage[0]);
			printf("ERROR: %s\n", &errorMessage[0]);
		}
		glDetachShader(id, vertex);
		glDetachShader(id, fragment);
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}
}

Shader::Shader(std::string comp) {
	Shader::Shader(comp, std::vector<std::string>());
}


Shader::Shader(std::string comp, std::vector<std::string> args) {
	std::string compString = readFile(comp);
	//Hook in to just after #version and inject our defines
	size_t secondLine = compString.find_first_of('\n');
	std::string defines = "";
	for (std::string arg : args) {
		defines += "#define " + arg + "\n";
	}
	compString.insert(secondLine + 1, defines);
	const char* compChars = compString.c_str();
	if (*compChars) {
		GLuint compute = glCreateShader(GL_COMPUTE_SHADER);
		//Compile compute shader
		glShaderSource(compute, 1, &compChars, NULL);
		glCompileShader(compute);
		//Check for errors
		GLint result = GL_FALSE;
		int logLength;
		glGetShaderiv(compute, GL_COMPILE_STATUS, &result);
		glGetShaderiv(compute, GL_INFO_LOG_LENGTH, &logLength);
		if (result != GL_TRUE || logLength > 0) {
			std::vector<char> errorMessage(logLength + 1);
			glGetShaderInfoLog(compute, logLength, NULL, &errorMessage[0]);
			printf("ERROR: %s\n", &errorMessage[0]);
		}
		id = glCreateProgram();
		glAttachShader(id, compute);
		glLinkProgram(id);
		glGetProgramiv(id, GL_LINK_STATUS, &result);
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logLength);
		if (result != GL_TRUE || logLength > 0) {
			std::vector<char> errorMessage(logLength + 1);
			glGetProgramInfoLog(id, logLength, NULL, &errorMessage[0]);
			printf("ERROR: %s\n", &errorMessage[0]);
		}
		glDetachShader(id, compute);
		glDeleteShader(compute);
	}
}

std::string Shader::readFile(std::string filename) {
	std::ifstream inStream(filename, std::ios::in);
	if (!inStream.is_open()) {
		return "";
	}
	std::string file;
	std::string line;
	while (std::getline(inStream, line))
		file += line + "\n";
	inStream.close();
	return file;
}

Shader::~Shader() {
	//glDeleteProgram(id);
}

void Shader::deleteShader() {
	glDeleteProgram(id);
	id = -1;
}
