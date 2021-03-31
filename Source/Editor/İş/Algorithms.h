#pragma once
#include "Editor/Editor_Includes.h"
#include "DataTypes.h"
#include "GFX/GFX_Core.h"

namespace TuranEditor {

	class Algorithms {
	public:
		static void Generate_KDTree(PointCloud& Cloud);
		static vec3 Searchfor_ClosestPoint(PointCloud& cloud, vec3& Point, long long* timing = nullptr);
		static vector<vec3> Searchfor_ClosestNeighbors(PointCloud& cloud, vec3& Point, unsigned int numberofneighbors, bool KDTreeSearch, long long* timing = nullptr);
		static vector<vec3> Compute_PCA(const vector<vec3>& points);
		static vector<vec3> ProjectPoints_OnPlane_thenTriangulate(const vector<vec3>& points, vec3 Tangent, vec3 Bitangent, vec3 Normal);
		static unsigned int ReconstructSurface(PointCloud* CLOUD, unsigned char KNearestNeighor, GFX_API::VertexAttributeLayout& PositionNormal_VertexAttrib);
	};

}