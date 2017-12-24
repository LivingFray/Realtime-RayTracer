//Returns whether there was a collision and updates Collision struct accordingly
bool getSphereCollision(Sphere s, vec3 rayOrigin, vec3 rayDirection, inout Collision c);
//Returns whether there exists a collision between the ray and the sphere
bool hasSphereCollision(Sphere s, vec3 rayOrigin, vec3 rayDirection);