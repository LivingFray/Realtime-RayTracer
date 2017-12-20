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
#define CAMERA_SPEED 0.1f
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

	if (horizontalAngle > glm::two_pi<double>()) {
		horizontalAngle -= glm::two_pi<double>();
	}
	if (horizontalAngle < 0.0) {
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
	float shininess;
};

struct Plane {
	glm::vec3 pos;
	float shininess;
	glm::vec3 norm;
	float paddingA;
	glm::vec3 colour;
	float paddingB;
};

struct Light {
	glm::vec3 pos;
	float isDirectional;
	glm::vec3 colour;
	float radius;
	float constant;
	float linear;
	float quadratic;
	//Pre calc maximum distance light is visible from
	float maxDist;
};


int inline getGrid(float pos, float size, float minCoord, int numGrids) {
	return glm::clamp(static_cast<int>(ceil((pos - minCoord) * size)), 0, numGrids - 1);
}

/*
In order to store a regular grid on a GPU using contiguous memory, grid points
to the first index of a list of vertices. Relevant primitves are determined by
accessing the primitives list from index currentGrid.value to (currentGrid+1).value

*/
#define DENSITY 8.0f
void generateGrid(std::vector<Sphere>& spheres, std::vector<int>& grid, std::vector<int>& lists, GLuint id) {
	//Determine bounding box for grid
	float minX, minY, minZ, maxX, maxY, maxZ;
	minX = minY = minZ = INFINITY;
	maxX = maxY = maxZ = -INFINITY;
	for (Sphere s : spheres) {
		if (s.pos.x - s.radius < minX) {
			minX = s.pos.x - s.radius;
		}
		if (s.pos.x + s.radius > maxX) {
			maxX = s.pos.x + s.radius;
		}
		if (s.pos.y - s.radius < minY) {
			minY = s.pos.y - s.radius;
		}
		if (s.pos.y + s.radius > maxY) {
			maxY = s.pos.y + s.radius;
		}
		if (s.pos.z - s.radius < minZ) {
			minZ = s.pos.z - s.radius;
		}
		if (s.pos.z + s.radius > maxZ) {
			maxZ = s.pos.z + s.radius;
		}
	}
	//Equation from http://www.dbd.puc-rio.br/depto_informatica/09_14_ivson.pdf
	float dX = maxX - minX;
	float dY = maxY - minY;
	float dZ = maxZ - minZ;
	float size = pow((DENSITY * spheres.size()) / (dX * dY * dZ), 1 / 3.0f);
	int nX = 2;//static_cast<int>(ceil(dX / size));
	int nY = 2;//static_cast<int>(ceil(dY / size));
	int nZ = 2;//static_cast<int>(ceil(dZ / size));
	float sizeX = dX / static_cast<float>(nX);
	float sizeY = dY / static_cast<float>(nY);
	float sizeZ = dZ / static_cast<float>(nZ);

	std::vector<std::vector<int>> gridSphereList;
	gridSphereList.resize(nX * nY * nZ);
	//For each sphere, test intersection with subsection of grid
	int spherePos = 0;
	for (Sphere s : spheres) {
		//Calculate boundaries of sphere
		int radGridSizeX = static_cast<int>(ceil(s.radius * sizeX)) - 1;
		int radGridSizeY = static_cast<int>(ceil(s.radius * sizeY)) - 1;
		int radGridSizeZ = static_cast<int>(ceil(s.radius * sizeZ)) - 1;
		int cX = getGrid(s.pos.x, sizeX, minX, nX);
		int cY = getGrid(s.pos.y, sizeY, minY, nY);
		int cZ = getGrid(s.pos.z, sizeZ, minZ, nZ);

		int xMin = static_cast<int>((((s.pos.x - s.radius) - minX) / sizeX));
		int yMin = static_cast<int>((((s.pos.y - s.radius) - minY) / sizeY));
		int zMin = static_cast<int>((((s.pos.z - s.radius) - minZ) / sizeZ));
		int xMax = static_cast<int>(ceil(((s.pos.x + s.radius) - minX) / sizeX));
		int yMax = static_cast<int>(ceil(((s.pos.y + s.radius) - minY) / sizeY));
		int zMax = static_cast<int>(ceil(((s.pos.z + s.radius) - minZ) / sizeZ));

		for (int x = xMin; x < xMax; x++) {
			for (int y = yMin; y < yMax; y++) {
				for (int z = zMin; z < zMax; z++) {
					gridSphereList[x + nX * y + nX * nY * z].push_back(spherePos);
				}
			}
		}
		//for (int x = cX - radGridSize; x < cX + radGridSize; x++) {
		/*for (int x = 0; x < nX; x++) {
			float gXMin = x * sizeX + minX;
			float gXMax = (x + 1) * sizeX + minX;
			//Sphere is too far away to possibly intersect
			if (xMax < gXMin || xMin > gXMax) {
				continue;
			}
			float closestX = glm::clamp(s.pos.x, gXMin, gXMax);
			//for (int y = cY - radGridSize; y < cY + radGridSize; y++) {
			for (int y = 0; y < nY; y++) {
				float gYMin = y * sizeY + minY;
				float gYMax = (y + 1) * sizeY + minY;
				//Sphere is too far away to possibly intersect
				if (yMax < gYMin || yMin > gYMax) {
					continue;
				}
				float closestY = glm::clamp(s.pos.y, gYMin, gYMax);
				//for (int z = cZ - radGridSize; z < cZ + radGridSize; z++) {
				for (int z = 0; z < nZ; z++) {
					float gZMin = z * sizeZ + minZ;
					float gZMax = (z + 1) * sizeZ + minZ;
					//Sphere is too far away to possibly intersect
					if (zMax < gZMin || zMin > gZMax) {
						continue;
					}
					float closestZ = glm::clamp(s.pos.z, gZMin, gZMax);
					//Test for overlap
					float distX = closestX - s.pos.x;
					float distY = closestY - s.pos.y;
					float distZ = closestZ - s.pos.z;
					if(distX * distX + distY * distY + distZ * distZ <= s.radius * s.radius) {
						gridSphereList[x + nX * y + nX * nY * z].push_back(spherePos);
					}
				}
			}
		}*/
		spherePos++;
	}
	grid.clear();
	lists.clear();
	//Take list of lists of grids and convert to GPU friendly format
	int currentIndex = 0;
	for (std::vector<int> spheres : gridSphereList) {
		//Calculate end index
		currentIndex += spheres.size();
		//Store end index
		grid.push_back(currentIndex);
		//For each sphere in the list, add to global sphere list
		for (int index : spheres) {
			lists.push_back(index);
		}
		//for (int i = 0; i < 4; i++) {
		//	lists.push_back(i);
		//}
	}
	glUseProgram(id);
	glUniform1i(glGetUniformLocation(id, "numX"), nX);
	glUniform1i(glGetUniformLocation(id, "numY"), nY);
	glUniform1i(glGetUniformLocation(id, "numZ"), nZ);
	glUniform1f(glGetUniformLocation(id, "sizeX"), sizeX);
	glUniform1f(glGetUniformLocation(id, "sizeY"), sizeY);
	glUniform1f(glGetUniformLocation(id, "sizeZ"), sizeZ);
	glUniform1f(glGetUniformLocation(id, "gridMinX"), minX);
	glUniform1f(glGetUniformLocation(id, "gridMinY"), minY);
	glUniform1f(glGetUniformLocation(id, "gridMinZ"), minZ);
	glUniform1f(glGetUniformLocation(id, "gridMaxX"), maxX);
	glUniform1f(glGetUniformLocation(id, "gridMaxY"), maxY);
	glUniform1f(glGetUniformLocation(id, "gridMaxZ"), maxZ);
	glUseProgram(0);
}

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
	int numSpheres = 2;
	for (int x = 0; x < numSpheres; x++) {
		for (int y = 0; y < numSpheres; y++) {
		struct Sphere newS;
		newS.pos = glm::vec3(static_cast<float>(x * 2) - numSpheres / 2.0f, 1.0f, static_cast<float>(y * 2) - numSpheres / 2.0f);
		newS.radius = 0.75f;
		newS.colour = glm::vec3(1.0f, 0.0f, 0.0f);
		newS.shininess = 64.0f;
		spheres.push_back(newS);
		}
	}
	std::vector<Plane> planes;
	{
		struct Plane newP;
		newP.pos = glm::vec3(0.0f, 0.0f, 0.0f);
		newP.norm = glm::vec3(0.0f, 1.0f, 0.0f);
		newP.colour = glm::vec3(0.0f, 1.0f, 0.0f);
		newP.shininess = 50.0f;
		planes.push_back(newP);
	}
	{
		struct Plane newP;
		newP.pos = glm::vec3(0.0f, 8.0f, 0.0f);
		newP.norm = glm::vec3(0.0f, -1.0f, 0.0f);
		newP.colour = glm::vec3(0.0f, 1.0f, 0.0f);
		newP.shininess = 50.0f;
		planes.push_back(newP);
	}
	std::vector<Light> lights;
	{
		struct Light newL;
		newL.pos = glm::vec3(0.0f, 4.0f, 0.0f);
		newL.colour = glm::vec3(1.0f, 1.0f, 1.0f);
		newL.constant = 1.0f;
		newL.linear = 0.22f;
		newL.quadratic = 0.2f;
		newL.isDirectional = 0.0f;
		newL.radius = 0.01f;
		newL.maxDist = 20.0f;
		lights.push_back(newL);
	}
	{
		struct Light newL;
		newL.pos = glm::vec3(0.0f, -1.0f, 0.0f);
		newL.colour = glm::vec3(0.3f, 0.3f, 0.3f);
		newL.isDirectional = 1.0f;
		lights.push_back(newL);
	}
	std::vector<int> grid;
	std::vector<int> list;
	generateGrid(spheres, grid, list, compute.getProgram());
	std::cout << "Generated grid (" << grid.size() << " nodes)" << std::endl;

	//Bind test input
	GLuint sphereSSBO;
	glGenBuffers(1, &sphereSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphereSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, spheres.size() * sizeof(Sphere), &spheres[0], GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sphereSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	GLuint gridSSBO;
	glGenBuffers(1, &gridSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, gridSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, grid.size() * sizeof(int), &grid[0], GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, gridSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	GLuint listSSBO;
	glGenBuffers(1, &listSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, listSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, list.size() * sizeof(int), &list[0], GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, listSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	GLuint planeSSBO;
	glGenBuffers(1, &planeSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, planeSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, planes.size() * sizeof(Plane), &planes[0], GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, planeSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	GLuint lightSSBO;
	glGenBuffers(1, &lightSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, lights.size() * sizeof(Light), &lights[0], GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, lightSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	//Set camera properties
	camPos = glm::vec3(2.0f, 4.0f, 2.0f);
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
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			glUseProgram(compute.getProgram());
			glUniform1i(glGetUniformLocation(compute.getProgram(), "debug"), 1);
			glUseProgram(0);
		} else {
			glUseProgram(compute.getProgram());
			glUniform1i(glGetUniformLocation(compute.getProgram(), "debug"), 0);
			glUseProgram(0);
		}
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