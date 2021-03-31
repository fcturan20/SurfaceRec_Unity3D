#pragma once
#include "Editor/Editor_Includes.h"

//Representation Index 0 is for KDTree
struct CloudRepresentation {
	void* DATA_ptr = nullptr;
	unsigned int RepresentationIndex = UINT32_MAX;
};
struct PointCloud {
	unsigned int PointCount = 0;
	vec3* PointPositions = nullptr, *PointNormals = nullptr;
	vector<CloudRepresentation> DifferentRepresentations;
};

