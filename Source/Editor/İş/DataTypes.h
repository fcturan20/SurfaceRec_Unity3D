#pragma once
#include "Editor/Editor_Includes.h"

//Representation Index 0 is for KDTree
//Representation Index 1 is for Type 1 Reconstruction Datas
//Representation Index 2 is for Type 2 Reconstruction Datas
struct CloudRepresentation {
	void* DATA_ptr = nullptr;
	unsigned int RepresentationIndex = UINT32_MAX;
};
struct PointCloud {
	unsigned int PointCount = 0;
	vec3* PointPositions = nullptr, *PointNormals = nullptr;
	vector<CloudRepresentation> DifferentRepresentations;
};

