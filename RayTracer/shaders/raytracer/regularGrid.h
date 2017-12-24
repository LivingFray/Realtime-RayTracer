//DDA variables
struct DDA {
	int stepX;
	int stepY;
	int stepZ;
	float deltaX;
	float deltaY;
	float deltaZ;
	int gridX;
	int gridY;
	int gridZ;
	float maxX;
	float maxY;
	float maxZ;
	bool firstSphereIt;
};

//Gets the next grid that contains a possible collision
bool getNextSphereList(inout DDA dda, inout int listStart, inout int listEnd);

//Resets the grid traversal algorithm
bool initSphereListRay(vec3 rayOrigin, vec3 rayDirection, inout DDA dda, inout int listStart, inout int listEnd);

//Returns if a ray intersects with the grid, setting the distance in dist if it does
bool distToAABB(vec3 rayOrigin, vec3 rayDirection, inout float dist);