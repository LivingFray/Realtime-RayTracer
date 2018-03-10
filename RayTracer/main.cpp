#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <vector>
#include <stdlib.h>
#include <fstream>
#include <memory>
#include <queue>
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glew32.lib")

#include "Shader.h"
#include "Simulation.h"
#include "Structures.h"


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

#define SAVE_TO_FILE

int main() {
	//Initialise OpenGL
	init(WIDTH, HEIGHT, "Ray Tracer");
	//initFullscreen("Ray Tracer");
	//Get number of work groups available
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &MAX_WORK_GROUPS[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &MAX_WORK_GROUPS[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &MAX_WORK_GROUPS[2]);
	std::cout << MAX_WORK_GROUPS[0] << ", " << MAX_WORK_GROUPS[1] << ", " << MAX_WORK_GROUPS[2] << " Work Groups available\n";

	//Simulation information
	/* Simulation arguments:
	 * DRAW_REGGRID - Draws the regular grid containing spheres
	 * DEBUG_REFLECT - Draws reflections in green
	 * DEBUG_REFRACT - Draws refractions in red
	 * NUM_SHADOW_RAYS [n] - Number of rays used to check for shadows
	 * MIN_CONTR [f] - How much a ray must contribute to the pixel colour to be explored
	 * MAX_RELFECT [n] - Maximum number of relfections calculated per pixel
	 * MAX_REFRACT [n] - Maximum number of refractions calculated per pixel
	 * MAX_DEPTH [n] - Maximum number of rays a single point can cast
	 * DONT_DRAW_LIGHTS - Draws shading from light sources (not shadows)
	 * To be implemented:
	 * EARLY_GRID_EXIT - Stops traversal of grid after a collision is found
	 * USE_GRID - Uses a regular grid to traverse sphere collisions
	 */
	std::queue<std::shared_ptr<Simulation>> sims;
	//Add simulations to run here
	for (int i = 1; i <= 10; i++) {
		std::shared_ptr<Simulation> s(new Simulation());
		s->DENSITY = i * 5.0;
		s->args = {
			"DRAW_REGGRID"
		};
		s->autoCamera = true;
		s->csv = std::to_string(s->DENSITY);
		sims.push(s);
	}

	//Set texture output
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

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
#ifdef SAVE_TO_FILE
	std::ofstream file;
	file.open("out.csv");
#endif
#define RECORD_LENGTH 100

	//Disable VSYNC
	glfwSwapInterval(0);

	//Hide cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	bool initNeeded = true;
	//Main render loop
	while (!glfwWindowShouldClose(window) && !glfwGetKey(window, GLFW_KEY_ESCAPE) && !sims.empty()) {
		//Calculate time elapsed
		time = glfwGetTime();
		dt = time - lastTime;
		lastTime = time;
		if (initNeeded && !sims.empty()) {
			sims.front()->init();
			dt = 0.0;
			initNeeded = false;
			//Prevent init from being recorded
			lastTime = glfwGetTime();
		}
		if (sims.front()->frames >= RECORD_LENGTH+3) {
			std::cout << "Minimum: " << sims.front()->minTime << "ms, Average: " << sims.front()->avgTime << "ms (" << (1000.0 / sims.front()->avgTime) << "), Maximum: " << sims.front()->maxTime << "ms" << std::endl;
#ifdef SAVE_TO_FILE
			//Save data
			file << sims.front()->csv << ',' << std::to_string(sims.front()->minTime) << ',' << std::to_string(sims.front()->avgTime) << ',' << std::to_string(sims.front()->maxTime) << std::endl;
#endif
			//Next simulation
			sims.pop();
			initNeeded = true;
		}

		//Poll events
		glfwPollEvents();
		if (!initNeeded) {
			//Tell current simulation to draw a frame
			sims.front()->run(dt);

			//Render result to screen
			glUseProgram(quad.getProgram());
			glBindVertexArray(vertexArray);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glBindVertexArray(0);
			glfwSwapBuffers(window);
		}
	}
#ifdef SAVE_TO_FILE
	file.flush();
	file.close();
#endif
	return 0;
}