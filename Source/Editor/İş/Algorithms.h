#pragma once
#include "Editor/Editor_Includes.h"
#include "DataTypes.h"
#include "GFX/GFX_Core.h"
#include "Editor/FileSystem/ResourceTypes/Model_Resource.h"

namespace TuranEditor {

	class Algorithms {
	public:
		static void Generate_KDTree(PointCloud& Cloud);
		static vec3 Searchfor_ClosestPoint(PointCloud& cloud, vec3& Point, long long* timing = nullptr);
		static vector<vec3> Searchfor_ClosestNeighbors(PointCloud& cloud, vec3& Point, unsigned int numberofneighbors, bool KDTreeSearch, long long* timing = nullptr);
		static vector<unsigned int> Searchfor_ClosestNeighbors(const PointCloud& cloud, vec3& Point, unsigned int numberofneighbors, long long* timing = nullptr);
		static vector<vec3> Compute_PCA(const vector<vec3>& points);
		static vector<vec3> ProjectPoints_OnPlane_thenTriangulate(const vector<vec3>& points, vec3 Tangent, vec3 Bitangent, vec3 Normal, vector<Triangle>* Faces = nullptr, vector<Edge>* Edges = nullptr);
        static bool rayTriangleIntersect(
            const vec3& orig, const vec3& dir,
            const vec3& v0, const vec3& v1, const vec3& v2,
            float& t);
		static vector<vec3> HoughBasedNormalReconstruction(const PointCloud& cloud, bool flip_normals = true, unsigned int K = 100, unsigned int T = 1000, unsigned int n_phi = 15, unsigned int n_rot = 5, bool ua = false, float tol_angle_rad = 0.79f, unsigned int k_density = 5);
		//Type 0 is a tangent plane for each vertex
		//Type 1 is a basic elimination of tangent planes
		//Type 2 is a complex elimination of tangent planes
		static unsigned int ReconstructSurface(PointCloud* CLOUD, unsigned char KNearestNeighor, GFX_API::VertexAttributeLayout& PositionNormal_VertexAttrib, unsigned char Type, const char* SavePath = nullptr);
		static unsigned int LoadReconstructedSurface_fromDisk(const char* PATH, GFX_API::VertexAttributeLayout& PositionNormal_VertexAttrib, unsigned char& KNN, unsigned char& Type);
	
	
	};

}