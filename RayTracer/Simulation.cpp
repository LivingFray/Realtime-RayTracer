#include "Simulation.h"
#include "Structures.h"
#include <iostream>

float randF(float min, float max) {
	float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	return r * (max - min) + min;
}

Simulation::Simulation() {
	camMat = glm::mat4(1.0f);
}


Simulation::~Simulation() {
	//Cleanup here
	shader.deleteShader();
}

void Simulation::init() {

	//Create compute shader
	shader = Shader("shaders/comp.glsl", args);

	//Create test input
	{
		//Opaque green
		struct Material newM;
		newM.colour = glm::vec3(0.2f, 0.7f, 0.1f);
		newM.opaque = 1;
		newM.refIndex = 1.5;
		newM.reflection = 0.0f;
		newM.shininess = 50.0f;
		materials.push_back(newM);
		//Reflective white
		newM.colour = glm::vec3(1.0f, 1.0f, 1.0f);
		newM.opaque = 1;
		newM.refIndex = 1.5;
		newM.reflection = 0.7f;
		newM.shininess = 50.0f;
		materials.push_back(newM);
		//Refractive white
		newM.colour = glm::vec3(1.0f, 1.0f, 1.0f);
		newM.opaque = 0;
		newM.refIndex = 1.05;
		newM.reflection = 0.0f;
		newM.shininess = 50.0f;
		materials.push_back(newM);
		//Reflective, refractive blue
		newM.colour = glm::vec3(0.0f, 0.3f, 1.0f);
		newM.opaque = 0;
		newM.refIndex = 1.05;
		newM.reflection = 0.5f;
		newM.shininess = 50.0f;
		materials.push_back(newM);
	}
	float minX = -5.0f;
	float minY = 0.0f;
	float minZ = -5.0f;
	float maxX = 5.0f;
	float maxY = 10.0f;
	float maxZ = 5.0f;
	for (int i = 0; i < numSpheres; i++) {
		struct Sphere newS;
		newS.pos = glm::vec3(randF(minX, maxX), randF(minY, maxY), randF(minZ, maxZ));
		newS.radius = 0.75f;
		newS.material = rand() % materials.size();
		spheres.push_back(newS);
	}
	{
		struct Plane newP;
		newP.pos = glm::vec3(0.0f, 0.0f, 0.0f);
		newP.norm = glm::vec3(0.0f, 1.0f, 0.0f);
		newP.material = 0;
		planes.push_back(newP);
	}
	for (int i = 0; i < numLights - 1; i++) {
		struct Light newL;
		newL.pos = glm::vec3(randF(minX * 2, maxX * 2), randF(minY * 2, maxY * 2), randF(minZ * 2, maxZ * 2));
		newL.colour = glm::vec3(1.0f, 1.0f, 1.0f);
		newL.constant = 1.0f;
		newL.linear = 0.22f;
		newL.quadratic = 0.2f;
		newL.isDirectional = 0.0f;
		newL.radius = 0.01f;
		newL.maxDist = 20.0f;
		lights.push_back(newL);
	}
	if (numLights > 0) {
		struct Light newL;
		newL.pos = glm::vec3(0.0f, -1.0f, 0.0f);
		newL.colour = glm::vec3(0.3f, 0.3f, 0.3f);
		newL.isDirectional = 1.0f;
		lights.push_back(newL);
	}
	{
		struct Triangle newT;
		newT.v1 = glm::vec3(-10.0f, -20.0f, -10.0f);
		newT.v2 = glm::vec3(10.0f, -20.0f, -10.0f);
		newT.v3 = glm::vec3(0.0f, -20.0f, 10.0f);
		newT.material = 0;
		triangles.push_back(newT);
	}
	generateGrid(spheres, grid, list, shader.getProgram());
	std::cout << "Generated grid (" << grid.size() << " nodes)" << std::endl;

	//Bind test input
	if (spheres.size() > 0) {
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
	}
	if (planes.size() > 0) {
		GLuint planeSSBO;
		glGenBuffers(1, &planeSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, planeSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, planes.size() * sizeof(Plane), &planes[0], GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, planeSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
	if (lights.size() > 0) {
		GLuint lightSSBO;
		glGenBuffers(1, &lightSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, lights.size() * sizeof(Light), &lights[0], GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, lightSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
	GLuint materialSSBO;
	glGenBuffers(1, &materialSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, materials.size() * sizeof(Material), &materials[0], GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, materialSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	if (triangles.size() > 0) {
		GLuint triangleSSBO;
		glGenBuffers(1, &triangleSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, triangles.size() * sizeof(Triangle), &triangles[0], GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, triangleSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	//Set camera properties
	camPos = glm::vec3(2.0f, 4.0f, 2.0f);
	glUseProgram(shader.getProgram());
	glUniform1f(glGetUniformLocation(shader.getProgram(), "twiceTanFovY"), tanf(3.1415926535f * FOV / 360.0f));
	glUniform1f(glGetUniformLocation(shader.getProgram(), "cameraWidth"), CAMERA_WIDTH);
	glUniform1f(glGetUniformLocation(shader.getProgram(), "cameraHeight"), CAMERA_HEIGHT);
	glUniformMatrix4fv(glGetUniformLocation(shader.getProgram(), "cameraMatrix"), 1, GL_FALSE, &camMat[0][0]);
	//Set output image
	glUniform1i(glGetUniformLocation(shader.getProgram(), "imgOut"), 0);
	glUseProgram(0);
}

void Simulation::run(double dt) {
	//Camera
	if (autoCamera) {
		autoUpdateCamera(dt);
	} else {
		manualUpdateCamera(dt);
	}
	double t = glfwGetTime();
	//Execute compute shader
	glUseProgram(shader.getProgram());
	glUniformMatrix4fv(glGetUniformLocation(shader.getProgram(), "cameraMatrix"), 1, GL_FALSE, &camMat[0][0]);
	//Ensure data is updated (use all barrier bits because I can't be bothered to check which exact ones I need)
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glDispatchCompute(WIDTH, HEIGHT, 1);
	//Wait for image to be fully generated
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	double elapsed = glfwGetTime() - t;
	csv += ", " + std::to_string(elapsed);
}

#define MOUSE_SPEED 0.005
#define CAMERA_SPEED 0.1f

void Simulation::manualUpdateCamera(double dt) {
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

void Simulation::autoUpdateCamera(double dt) {
	ang += dt;
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	camMat = glm::rotate(glm::mat4(1.0), static_cast<float>(ang), up) * glm::translate(glm::mat4(1), centre) * glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, -dist));
}


/*
In order to store a regular grid on a GPU using contiguous memory, grid points
to the first index of a list of vertices. Relevant primitves are determined by
accessing the primitives list from index currentGrid.value to (currentGrid+1).value

*/
void Simulation::generateGrid(std::vector<Sphere>& spheres, std::vector<int>& grid, std::vector<int>& lists, GLuint id) {
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
	//Give a small amount of space around the edges for rounding errors and such
	minX -= 0.05f;
	maxX += 0.05f;
	minY -= 0.05f;
	maxY += 0.05f;
	minZ -= 0.05f;
	maxZ += 0.05f;
	//Equation from http://www.dbd.puc-rio.br/depto_informatica/09_14_ivson.pdf
	//Get volume and calculate average size that holds 1 sphere
	float dX = maxX - minX;
	float dY = maxY - minY;
	float dZ = maxZ - minZ;
	float size = pow((DENSITY * spheres.size()) / (dX * dY * dZ), 1 / 3.0f);
	int nX = static_cast<int>(ceil(dX * size));
	int nY = static_cast<int>(ceil(dY * size));
	int nZ = static_cast<int>(ceil(dZ * size));
	float sizeX = dX / static_cast<float>(nX);
	float sizeY = dY / static_cast<float>(nY);
	float sizeZ = dZ / static_cast<float>(nZ);

	std::vector<std::vector<int>> gridSphereList;
	gridSphereList.resize(nX * nY * nZ);
	//For each sphere, test intersection with subsection of grid
	int spherePos = 0;
	for (Sphere s : spheres) {
		//Calculate boundaries of sphere
		int xMin = static_cast<int>(((s.pos.x - s.radius) - minX) / sizeX);
		int yMin = static_cast<int>(((s.pos.y - s.radius) - minY) / sizeY);
		int zMin = static_cast<int>(((s.pos.z - s.radius) - minZ) / sizeZ);
		int xMax = static_cast<int>(((s.pos.x + s.radius) - minX) / sizeX);
		int yMax = static_cast<int>(((s.pos.y + s.radius) - minY) / sizeY);
		int zMax = static_cast<int>(((s.pos.z + s.radius) - minZ) / sizeZ);
		for (int x = xMin; x <= xMax; x++) {
			for (int y = yMin; y <= yMax; y++) {
				for (int z = zMin; z <= zMax; z++) {
					gridSphereList[x + nX * y + nX * nY * z].push_back(spherePos);
				}
			}
		}
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

