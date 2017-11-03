#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <vector>
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glew32.lib")

#include "Shader.h"

#define GROUP_SIZE 1

#define WIDTH 1280
#define HEIGHT 720
#define CAMERA_WIDTH 16.0f
#define CAMERA_HEIGHT 9.0f

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

double horizontalAngle = 3.1415926, verticalAngle = 0.0;
bool firstMove = true;
glm::vec3 camPos;
#define MOUSE_SPEED 0.005
#define CAMERA_SPEED 1.0f
glm::mat4 camMat(1.0f);


void updateCamera(double dt) {
	double mx, my;
	int w, h;
	glfwGetCursorPos(window, &mx, &my);
	glfwGetWindowSize(window, &w, &h);
	mx -= static_cast<double>(w) / 2.0;
	my -= static_cast<double>(h) / 2.0;
	glfwSetCursorPos(window,
		static_cast<double>(w) / 2.0,
		static_cast<double>(h) / 2.0
	);
	if (firstMove) {
		mx = 0.0;
		my = 0.0;
		firstMove = false;
	}

	horizontalAngle += mx * MOUSE_SPEED;
	verticalAngle += my * MOUSE_SPEED;

	if (horizontalAngle  > glm::two_pi<double>()) {
		horizontalAngle -= glm::two_pi<double>();
	}
	if (horizontalAngle  < 0.0) {
		horizontalAngle += glm::two_pi<double>();
	}
	if (verticalAngle > glm::half_pi<double>()) {
		verticalAngle = glm::half_pi<double>();
	}
	if (verticalAngle < -glm::half_pi<double>()) {
		verticalAngle = -glm::half_pi<double>();
	}

	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);

	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);

	glm::vec3 up = glm::cross(right, direction);

	if (glfwGetKey(window, GLFW_KEY_I)) {
		camPos -= direction * CAMERA_SPEED;
	}
	if (glfwGetKey(window, GLFW_KEY_K)) {
		camPos += direction * CAMERA_SPEED;
	}
	if (glfwGetKey(window, GLFW_KEY_J)) {
		camPos -= right * CAMERA_SPEED;
	}
	if (glfwGetKey(window, GLFW_KEY_L)) {
		camPos += right * CAMERA_SPEED;
	}
	//Because of how I apply the view matrix it must be inverted
	camMat = glm::inverse(glm::lookAt(camPos, camPos + direction, up));
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
	for (int x = 0; x < 5; x++) {
		for (int y = 0; y < 5; y++) {
			struct Sphere newS;
			newS.pos = glm::vec3(static_cast<float>(x * 2)-2.5f, static_cast<float>(y * 2)-2.5f, 10.0f);
			newS.radius = 1.0f;//static_cast<float>(x + y) / 20.0f;
			newS.colour = glm::vec3(static_cast<float>(x)/5.0f, static_cast<float>(y)/5.0f, 0.0f);
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
	glUniform1f(glGetUniformLocation(compute.getProgram(), "cameraWidth"), CAMERA_WIDTH);
	glUniform1f(glGetUniformLocation(compute.getProgram(), "cameraHeight"), CAMERA_HEIGHT);
	glUniformMatrix4fv(glGetUniformLocation(compute.getProgram(), "cameraMatrix"), 1, GL_FALSE, &camMat[0][0]);
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
	int frames = 0;
	double timeSincePrinted = 0;
#define FRAME_EVERY 10.0

	//Disable VSYNC
	glfwSwapInterval(0);

	//Hide cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	//Main render loop
	while (!glfwWindowShouldClose(window) && !glfwGetKey(window, GLFW_KEY_ESCAPE)) {
		//Calculate time elapsed
		time = glfwGetTime();
		dt = time - lastTime;
		lastTime = time;

		//Print Frames per second
		frames++;
		timeSincePrinted += dt;
		if (timeSincePrinted > FRAME_EVERY) {
			std::cout << static_cast<float>(frames) / FRAME_EVERY << std::endl;
			frames = 0;
			timeSincePrinted -= FRAME_EVERY;
		}

		//Poll events
		glfwPollEvents();

		//Camera
		updateCamera(dt);

		//Execute compute shader
		glUseProgram(compute.getProgram());
		glUniformMatrix4fv(glGetUniformLocation(compute.getProgram(), "cameraMatrix"), 1, GL_FALSE, &camMat[0][0]);
		//Ensure data is updated (use all barrier bits because I can't be bothered to check which exact ones I need)
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
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