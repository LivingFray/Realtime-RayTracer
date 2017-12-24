//Returns whether there was a collision and updates Collision struct accordingly
bool getPlaneCollision(Plane p, vec3 rayOrigin, vec3 rayDirection, inout Collision c);
//Returns whether the plane intersects with the ray
bool hasPlaneCollision(Plane p, vec3 rayOrigin, vec3 rayDirection, float minDist, float maxDist);