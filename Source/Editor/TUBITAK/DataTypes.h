#pragma once
#include "Editor/Editor_Includes.h"

//Representation Index 0 is for KDTree
//Representation Index 1 is for Type 1 Reconstruction Datas
//Representation Index 2 is for Type 2 Reconstruction Datas
struct CloudRepresentation {
	void* DATA_ptr = nullptr;
	unsigned int RepresentationIndex = UINT32_MAX;
};
//Original normals are imported with ORIGINAL_NORMALNAME name, all other normals should have a different NAME
struct PC_PointNormals {
	vec3* Normals;
	string NAME;
	static constexpr const char* ORIGINAL_NORMALNAME = "Original";
};

struct PointCloud {
	unsigned int PointCount = 0;
	vec3* PointPositions = nullptr;
	vector<PC_PointNormals> PointNormals;
	vector<CloudRepresentation> DifferentRepresentations;
};

