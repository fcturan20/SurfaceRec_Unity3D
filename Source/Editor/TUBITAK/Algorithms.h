#pragma once
#include "Editor/Editor_Includes.h"
#include "DataTypes.h"
#include "GFX/GFX_Core.h"
#include "Editor/FileSystem/ResourceTypes/Model_Resource.h"
#include "Editor/RenderContext/Editor_DataManager.h"

namespace TuranEditor {

	class Algorithms {
	public:
		static void Generate_KDTree(PointCloud& Cloud);
		static vec3 Searchfor_ClosestPoint(const PointCloud& cloud, const vec3& Point, long long* timing = nullptr);
		static vector<vec3> Searchfor_ClosestNeighbors(PointCloud& cloud, vec3& Point, unsigned int numberofneighbors, bool KDTreeSearch, long long* timing = nullptr);
		static vector<unsigned int> Searchfor_ClosestNeighbors(const PointCloud& cloud, const vec3& Point, unsigned int numberofneighbors, long long* timing = nullptr);
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
	
		static void HoughBasedSphereDetection(PointCloud* CLOUD, double MinSphereRadius, unsigned int BoundingCubeRes, unsigned int MaximaThreshold, std::vector<glm::vec4>& FoundSpheres);
		struct Plane {
			vec3 Center = {}, Tangent = {}, Bitangent = {};
			vec2 Size = {};
		};



		static void HoughBasedPlaneDetection(PointCloud* CLOUD, unsigned int DistanceSampleCount, unsigned int AngleSampleCount, unsigned int MaximaThreshold, std::vector<Plane>& FoundPlanes);
		//Returns true when all the work is finished. Returns false if there is still any work.
		static bool Progressive_HoughBasedPlaneDetection(PointCloud* CLOUD, unsigned int BoundingCubeRes, unsigned int AngleSampleCount,
			unsigned int MaximaThreshold, unsigned int& Step, bool ShouldContinue, vector<GFX_API::SpecialDrawCall>& SpecialDrawCallBuffer, TuranEditor::POINTRENDERER* PCRenderer);
		
		//Bounded Voronoi diagram (Bounded means connections to infinity is removed)
		struct VoronoiDiagram {
			struct VoronoiRegion {
				unsigned int* VertexIDs = nullptr, VertexCount = 0;
				vector<unsigned int> EdgeIDs, TriangleIDs;
			};
			struct VoronoiEdge {
				vec3 EdgePoints[2] = { vec3(FLT_MAX, FLT_MAX, FLT_MAX) };
			};
			struct VoronoiTriangle {
				vec3 Vertexes[3] = { vec3(FLT_MAX) };
			};
			//VoronoiEdges isn't filled for now!
			std::vector<VoronoiEdge> VoronoiEdges;
			std::vector<VoronoiRegion> VoronoiRegions;
			std::vector<VoronoiTriangle> VoronoiTriangles;
			//Positions of voronoi vertexes
			std::vector<vec3> VoronoiVertices;
			//Returns the ID of the edge
			//unsigned int AddVoronoiEdge(unsigned int EdgePoint0, unsigned int EdgePoint1);
		};
		static VoronoiDiagram* CreateVoronoiDiagram(PointCloud* CLOUD);

		//If you want to visualize SDF volume, you may want to sample locations. SampleLocations will clear and reallocate the given vector.
		//SampleLocations and the returned vector<float> vectors have the same order
		//If a SDF value is FLT_MAX, it is inside far away. If negative FLT_MAX, vice versa.
		static vector<float> SDF_FromPointCloud(PointCloud* CLOUD, uvec3 SDFCubeRes, unsigned int NormalListIndex, float SamplingD, vector<vec3>* SampleLocations = nullptr);
		static vector<float> SDF_FromVertexBuffer(const std::vector<vec3>& VertexPositions, const std::vector<vec3> VertexNormals, uvec3 SDFCubeRes, vec3 BOUNDINGMIN, vec3 BOUNDINGMAX, vector<vec3>* SampleLocations = nullptr, const PointCloud* const RefPC = nullptr, unsigned int RefPC_NormalIndex = UINT32_MAX, float SamplingD = 1.0f);
		//Positive is inside, negative is outside, FLT_MAX is further than sample cube diagonal
		//If you need vertex normals, you can give a reference point cloud which is used to verify calculated normals
		static vector<vec3> MarchingCubes(uvec3 SDFResolution, const vector<float>& SDFs, const vector<vec3>& SamplePositions, const PointCloud* RefPC = nullptr, vector<vec3>* OutputNormals = nullptr);

	};

}