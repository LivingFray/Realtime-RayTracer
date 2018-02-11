bool distToAABB(vec3 rayOrigin, vec3 rayDirection, inout float dist) {
	vec3 dirInv = vec3(1.0 / rayDirection.x, 1.0 / rayDirection.y, 1.0 / rayDirection.z);
    float tx1 = (gridMinX - rayOrigin.x)*dirInv.x;
    float tx2 = (gridMaxX - rayOrigin.x)*dirInv.x;
 
    float tmin = min(tx1, tx2);
    float tmax = max(tx1, tx2);
 
    float ty1 = (gridMinY - rayOrigin.y)*dirInv.y;
    float ty2 = (gridMaxY - rayOrigin.y)*dirInv.y;
 
    tmin = max(tmin, min(ty1, ty2));
    tmax = min(tmax, max(ty1, ty2));
	
	float tz1 = (gridMinZ - rayOrigin.z)*dirInv.z;
    float tz2 = (gridMaxZ - rayOrigin.z)*dirInv.z;
 
    tmin = max(tmin, min(tz1, tz2));
    tmax = min(tmax, max(tz1, tz2));
	
	dist = tmin;
    return tmax >= tmin && tmax > 0.0;
}

bool initSphereListRay(vec3 rayOrigin, vec3 rayDirection, inout DDA dda, inout int listStart, inout int listEnd) {
	listStart = 0;
	listEnd = 0;
	dda.stepX = rayDirection.x > 0 ? 1 : -1;
	dda.stepY = rayDirection.y > 0 ? 1 : -1;
	dda.stepZ = rayDirection.z > 0 ? 1 : -1;

	//Keep ray in bounds
	float dist;
	if(distToAABB(rayOrigin, rayDirection, dist)){
		rayOrigin = rayOrigin + dist * rayDirection;
	} else {
		return false;
	}
	
	dda.deltaX = sizeX / rayDirection.x;
	dda.deltaY = sizeY / rayDirection.y;
	dda.deltaZ = sizeZ / rayDirection.z;
	
	//Make negative directions positive
	dda.deltaX *= dda.stepX;
	dda.deltaY *= dda.stepY;
	dda.deltaZ *= dda.stepZ;
	
	dda.gridX = clamp(int((rayOrigin.x - gridMinX) / sizeX), 0, numX - 1);
	dda.gridY = clamp(int((rayOrigin.y - gridMinY) / sizeY), 0, numY - 1);
	dda.gridZ = clamp(int((rayOrigin.z - gridMinZ) / sizeZ), 0, numZ - 1);
	dda.maxX = ((dda.gridX + max(0, dda.stepX))*sizeX + gridMinX - rayOrigin.x) / rayDirection.x;
	dda.maxY = ((dda.gridY + max(0, dda.stepY))*sizeY + gridMinY - rayOrigin.y) / rayDirection.y;
	dda.maxZ = ((dda.gridZ + max(0, dda.stepZ))*sizeZ + gridMinZ - rayOrigin.z) / rayDirection.z;
	dda.firstSphereIt = true;
	if(dda.maxX < dda.maxY) {
		if(dda.maxX < dda.maxZ) {
			dda.distToEdge = dda.maxX;
		} else {
			dda.distToEdge = dda.maxZ;
		}
	} else {
		if(dda.maxY < dda.maxZ) {
			dda.distToEdge = dda.maxY;
		} else {
			dda.distToEdge = dda.maxZ;
		}
	}
	return true;
}
/*
Store index of last sphere
Get first index from previous
If no previous first is 0

LIST_START = Index of first relevant sphere in sphereList
LIST_END = Index of first non relevant sphere in sphereList

If no matches LIST_START = LIST_END

*/
bool getNextSphereList(inout DDA dda, inout int listStart, inout int listEnd) {
	int index;
	if(!dda.firstSphereIt){
		if(dda.maxX < dda.maxY) {
			if(dda.maxX < dda.maxZ) {
				dda.gridX = dda.gridX + dda.stepX;
				if(dda.gridX == numX || dda.gridX == -1)
					return false; 
				dda.maxX = dda.maxX + dda.deltaX;
				dda.distToEdge += dda.deltaX;
			} else  {
				dda.gridZ = dda.gridZ + dda.stepZ;
				if(dda.gridZ == numZ || dda.gridZ == -1)
					return false;
				dda.maxZ = dda.maxZ + dda.deltaZ;
				dda.distToEdge += dda.deltaZ;
			}
		} else  {
			if(dda.maxY < dda.maxZ) {
				dda.gridY = dda.gridY + dda.stepY;
				if(dda.gridY == numY || dda.gridY == -1)
					return false;
				dda.maxY = dda.maxY + dda.deltaY;
				dda.distToEdge += dda.deltaY;
			} else  {
				dda.gridZ = dda.gridZ + dda.stepZ;
				if(dda.gridZ == numZ || dda.gridZ == -1)
					return false;
				dda.maxZ = dda.maxZ + dda.deltaZ;
				dda.distToEdge += dda.deltaZ;
			}
		}
	}
	dda.firstSphereIt = false;
	index = dda.gridX + dda.gridY * numX + dda.gridZ * numX * numY;
	listStart = min(index, 1) * sphereGrid[index - 1];
	listEnd = sphereGrid[index];
	return true;
}