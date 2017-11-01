#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm\glm.hpp>
#include <vector>
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glew32.lib")

#include "Shader.h"

#define GROUP_SIZE 1

#define WIDTH 800
#define HEIGHT 600

#define FOV 90

//Use dedicated graphics card
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

GLFWwindow* window = nullptr;

int MAX_WORK_GROUPS[3];

void sharedInit() {
	if (!window) {
		std::cerr << "Could not create window" << std::endl;
		glfwTerminate();
		exit(1);
	}
	//New OpenGL only
	glfwWindowHint(GLFW_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwMakeContextCurrent(window);
	// Initialize GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		std::cerr << "Could not initialise GLEW" << std::endl;
		glfwTerminate();
		exit(1);
	}
	//Useful debug info about the graphics card in use
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl
		<< "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl
		<< "Vendor: " << glGetString(GL_VENDOR) << std::endl;
}

void init(int width = 800, int height = 600, const char* title = "Window",
	GLFWmonitor* monitor = NULL, GLFWwindow* share = NULL) {
	if (!glfwInit()) {
		std::cerr << "Could not initialise glfw" << std::endl;
		exit(1);
	}
	window = glfwCreateWindow(width, height, title, monitor, share);
	sharedInit();
}

void initFullscreen(const char* title = "Window", GLFWmonitor* monitor = NULL) {
	if (!glfwInit()) {
		std::cerr << "Could not initialise glfw" << std::endl;
		exit(1);
	}
	if (monitor == nullptr) {
		monitor = glfwGetPrimaryMonitor();
	}
	//Google says this is how you enter borderless windowed mode
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	window = glfwCreateWindow(mode->width, mode->height, title, monitor, NULL);
	sharedInit();
}

void destroyOpenGL() {
	if (window) {
		glfwDestroyWindow(window);
	}
	glfwTerminate();
}

void generateQuad(GLuint *vertexArray, GLuint *vertexBuffer, GLuint *textureBuffer) {
	//Generate array and buffers
	glGenVertexArrays(1, vertexArray);
	glGenBuffers(1, vertexBuffer);
	glGenBuffers(1, textureBuffer);
	GLfloat vertices[] = {
		-1.0f, 1.0f,
		-1.0f, -1.0f,
		1.0f, 1.0f,
		1.0f, -1.0f
	};
	GLfloat textures[] = {
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 1.0f,
		1.0f, 0.0f
	};
	glBindVertexArray(*vertexArray);
	//Pass vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, *vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	//Pass uvs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, *textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textures), &textures, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glBindVertexArray(0);
}

//Due to the wonders of byte alignment pad everything to be 4bytes
struct Sphere {
	glm::vec3 pos;
	float radius;
	glm::vec3 colour;
	float padding;
};


int main() {
	//Initialise OpenGL
	init(WIDTH, HEIGHT, "Ray Tracer");

	//Get number of work groups available
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &MAX_WORK_GROUPS[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &MAX_WORK_GROUPS[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &MAX_WORK_GROUPS[2]);
	std::cout << MAX_WORK_GROUPS[0] << ", " << MAX_WORK_GROUPS[1] << ", " << MAX_WORK_GROUPS[2] << " Work Groups available\n";

	//Create compute shader
	Shader compute("shaders/comp.glsl");

	//Create test input
	std::vector<Sphere> spheres;
	for (int x = 0; x < 11; x++) {
		for (int y = 0; y < 11; y++) {
			struct Sphere newS;
			newS.pos = glm::vec3(static_cast<float>(x)-5.0f, static_cast<float>(y)-5.0f, 10.0f);
			newS.radius = 1.0f;//static_cast<float>(x + y) / 20.0f;
			newS.colour = glm::vec3(static_cast<float>(x)/10.0f, static_cast<float>(y)/10.0f, 0.0f);
			spheres.push_back(newS);
		}
	}

	//Bind test input
	GLuint sphereSSBO;
	glGenBuffers(1, &sphereSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphereSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, spheres.size() * sizeof(Sphere), &spheres[0], GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, sphereSSBO);

	//Set camera properties
	glUseProgram(compute.getProgram());
	glUniform1f(glGetUniformLocation(compute.getProgram(), "twiceTanFovY"), tanf(3.1415926535f * FOV / 360.0f));
	glUniform1f(glGetUniformLocation(compute.getProgram(), "cameraWidth"), 4.0f);
	glUniform1f(glGetUniformLocation(compute.getProgram(), "cameraHeight"), 3.0f);
	glUseProgram(0);

	//Set texture output
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glUseProgram(compute.getProgram());
	glUniform1i(glGetUniformLocation(compute.getProgram(), "imgOut"), 0);
	glUseProgram(0);

	//Create Quad output
	GLuint vertexArray, vertexBuffer, textureBuffer;
	generateQuad(&vertexArray, &vertexBuffer, &textureBuffer);
	Shader quad("shaders/quad.vert", "shaders/quad.frag");
	glUseProgram(quad.getProgram());
	glUniform1f(glGetUniformLocation(quad.getProgram(), "imgOut"), 0);
	glUseProgram(0);

	//Ensure correct texture is being used for output
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);

	//Timing variables
	double time = 0, lastTime = 0, dt = 0;

	//Main render loop
	while (!glfwWindowShouldClose(window) && !glfwGetKey(window, GLFW_KEY_ESCAPE)) {
		//Calculate time elapsed
		time = glfwGetTime();
		dt = time - lastTime;
		lastTime = time;
		//Poll events
		glfwPollEvents();
		//Execute compute shader
		glUseProgram(compute.getProgram());
		glDispatchCompute(WIDTH, HEIGHT, 1);
		//Wait for image to be fully generated
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		//Render result to screen
		glUseProgram(quad.getProgram());
		glBindVertexArray(vertexArray);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
		glfwSwapBuffers(window);
	}
	return 0;
}