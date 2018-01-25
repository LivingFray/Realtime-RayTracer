//Returns whether there was a collision and updates Collision struct accordingly
bool getTriangleCollision(Triangle t, vec3 rayOrigin, vec3 rayDirection, inout Collision col);
//Returns whether there exists a collision between the ray and the triangle
bool hasTriangleCollision(Triangle t, vec3 rayOrigin, vec3 rayDirection, float minDist, float maxDist);