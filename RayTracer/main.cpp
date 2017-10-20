#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glew32.lib")

//Use dedicated graphics card
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

GLFWwindow* window = nullptr;

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

int main() {
	init(800, 600, "Ray Tracer");
	while (!glfwWindowShouldClose(window) && !glfwGetKey(window, GLFW_KEY_ESCAPE)) {
		glfwPollEvents();
		//Do compute shader things

		//Render result to screen
		glfwSwapBuffers(window);
	}
	return 0;
}