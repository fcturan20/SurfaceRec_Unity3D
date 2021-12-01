#include "Algorithms.h"
#include "TuranAPI/Logger_Core.h"
#include "TuranAPI/Profiler_Core.h"
#include <array>
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include "3rdPartyLibs/Normals.h"
#include "delaunator.hpp"
#include "GFX/GFX_Core.h"
#include "Editor/RenderContext/Editor_DataManager.h"
#include "Editor/RenderContext/Game_RenderGraph.h"
#include "Editor/FileSystem/DataFormats/JobDatas_generated.h"
using namespace TuranAPI;

#include "kdtree.h"

class MyPoint : public ::std::array<float, 3>
{
public:
	// dimension of the Point
	static const int DIM = 3;
};
vec3 BruteForce_ClosestPoint(vec3 point, PointCloud& Cloud, long long* timing = nullptr) {
	if (!timing) {
		timing = new long long;
	}
	TURAN_PROFILE_SCOPE_O("Brute Force Closest Search", timing);
	float MINDIST = FLT_MAX;
	vec3 closestpoint(FLT_MAX, FLT_MAX, FLT_MAX);
	for (unsigned int PointIndex = 0; PointIndex < Cloud.PointCount; PointIndex++) {
		vec3& SearchPoint = Cloud.PointPositions[PointIndex];
		float dist = length(point - SearchPoint);
		if (dist < MINDIST) {
			MINDIST = dist;
			closestpoint = SearchPoint;
		}
	}
	return closestpoint;
}
vector<vec3> BruteForce_KNearestPoint(vec3 point, PointCloud& Cloud, unsigned int KNeighbourCount, long long* timing = nullptr) {
	if (!timing) {
		timing = new long long;
	}
	TURAN_PROFILE_SCOPE_O("Brute Force KNearest Search", timing);
	vector<float> MINDISTs(KNeighbourCount, FLT_MAX);
	vector<vec3> closestpoints(KNeighbourCount, vec3(FLT_MAX, FLT_MAX, FLT_MAX));
	for (unsigned int PointIndex = 0; PointIndex < Cloud.PointCount; PointIndex++) {
		vec3& SearchPoint = Cloud.PointPositions[PointIndex];
		float dist = length(point - SearchPoint);
		if (dist < MINDISTs[0]) {
			MINDISTs[0] = dist;
			closestpoints[0] = SearchPoint;
		}
	}
	for (unsigned int NeighborSearchIndex = 1; NeighborSearchIndex < KNeighbourCount; NeighborSearchIndex++) {
		for (unsigned int PointIndex = 0; PointIndex < Cloud.PointCount; PointIndex++) {
			vec3& SearchPoint = Cloud.PointPositions[PointIndex];
			float dist = length(point - SearchPoint);
			if (dist < MINDISTs[NeighborSearchIndex]) {
				if (dist > MINDISTs[NeighborSearchIndex - 1]) {
					MINDISTs[NeighborSearchIndex] = dist;
					closestpoints[NeighborSearchIndex] = SearchPoint;
				}
			}
		}
	}

	return closestpoints;
}

void TuranEditor::Algorithms::Generate_KDTree(PointCloud& Cloud) {
	if (!Cloud.PointCount) {
		LOG_ERROR("Cloud is empty!");
		return;
	}

	for (unsigned int i = 0; i < Cloud.DifferentRepresentations.size(); i++) {
		if (Cloud.DifferentRepresentations[i].RepresentationIndex == 0) {
			return;
		}
	}

	CloudRepresentation cloud_representation;
	vector<MyPoint> NewCloud;
	for (unsigned int PointIndex = 0; PointIndex < Cloud.PointCount; PointIndex++) {
		MyPoint point;
		point[0] = Cloud.PointPositions[PointIndex].x;
		point[1] = Cloud.PointPositions[PointIndex].y;
		point[2] = Cloud.PointPositions[PointIndex].z;
		NewCloud.push_back(point);
	}
	kdt::KDTree<MyPoint>* final_cloud = new kdt::KDTree<MyPoint>(NewCloud);
	cloud_representation.DATA_ptr = final_cloud;
	cloud_representation.RepresentationIndex = 0;
	Cloud.DifferentRepresentations.push_back(cloud_representation);
}
vec3 TuranEditor::Algorithms::Searchfor_ClosestPoint(const PointCloud& cloud, const vec3& Point, long long* timing) {
	if (!timing) {
		timing = new long long;
	}
	TURAN_PROFILE_SCOPE_O("Searchfor_ClosestPoint", timing);


	/*Google KDTree
	kdtree* kd = (kdtree*)kd_ptr;
	kdres* result = kd_nearest3f(kd, Point.x, Point.y, Point.z);
	kd_res_item3f(result, &value.x, &value.y, &value.z);*/

	/*
	KDTree* kd = (KDTree*)kd_ptr;
	point_t x = kd->nearest_point({ Point.x, Point.y, Point.z });
	value.x = x[0];
	value.y = x[1];
	value.z = x[2];
	*/

	kdt::KDTree<MyPoint>* kdtree = nullptr;
	for (unsigned int CloudDataIndex = 0; CloudDataIndex < cloud.DifferentRepresentations.size(); CloudDataIndex++) {
		if (cloud.DifferentRepresentations[CloudDataIndex].RepresentationIndex == 0) {
			kdtree = (kdt::KDTree<MyPoint>*)cloud.DifferentRepresentations[CloudDataIndex].DATA_ptr;
		}
	}
	if (!kdtree) {
		LOG_CRASHING("KDTree is not built, so search has failed!");
	}
	MyPoint querypoint;
	querypoint[0] = Point.x;
	querypoint[1] = Point.y;
	querypoint[2] = Point.z;
	int index = kdtree->nnSearch(querypoint);
	if (index >= 0) {
		return cloud.PointPositions[index];
	}
	else {
		LOG_CRASHING("KDTree search has failed, returned index is negative!");
	}



	return vec3(UINT32_MAX, UINT32_MAX, UINT32_MAX);
}
vector<vec3> TuranEditor::Algorithms::Searchfor_ClosestNeighbors(PointCloud& cloud, vec3& Point, unsigned int numberofneighbors, bool KDTreeSearch, long long* timing) {
	if (!timing) {
		timing = new long long;
	}
	TURAN_PROFILE_SCOPE_O("Searchfor_ClosestNeighbors", timing);

	vector<vec3> final_positions;
	final_positions.resize(numberofneighbors);
	if (KDTreeSearch) {
		kdt::KDTree<MyPoint>* kdtree = nullptr;
		for (unsigned int CloudDataIndex = 0; CloudDataIndex < cloud.DifferentRepresentations.size(); CloudDataIndex++) {
			if (cloud.DifferentRepresentations[CloudDataIndex].RepresentationIndex == 0) {
				kdtree = (kdt::KDTree<MyPoint>*)cloud.DifferentRepresentations[CloudDataIndex].DATA_ptr;
			}
		}
		if (!kdtree) {
			LOG_CRASHING("KDTree is not built, so search has failed!");
		}
		MyPoint querypoint;
		querypoint[0] = Point.x;
		querypoint[1] = Point.y;
		querypoint[2] = Point.z;
		vector<int> indices = kdtree->knnSearch(querypoint, numberofneighbors);
		for (unsigned int index_i = 0; index_i < indices.size(); index_i++) {
			if (indices[index_i] < 0) {
				LOG_CRASHING("KDTree search has failed, returned index is negative!");
			}
		}
		if (numberofneighbors != indices.size()) {
			LOG_CRASHING("Number of neighbors wanted doesn't match number of neighbors found!");
		}
		for (unsigned int i = 0; i < indices.size(); i++) {
			final_positions[i] = cloud.PointPositions[indices[i]];
		}
	}
	else {
		final_positions = BruteForce_KNearestPoint(Point, cloud, numberofneighbors);
	}


	return final_positions;
}
vector<unsigned int> TuranEditor::Algorithms::Searchfor_ClosestNeighbors(const PointCloud& cloud, const vec3& Point, unsigned int numberofneighbors, long long* timing) {
	if (!timing) {
		timing = new long long;
	}
	TURAN_PROFILE_SCOPE_O("Searchfor_ClosestNeighbors", timing);

	vector<unsigned int> final_positions;
	final_positions.resize(numberofneighbors);


	kdt::KDTree<MyPoint>* kdtree = nullptr;
	for (unsigned int CloudDataIndex = 0; CloudDataIndex < cloud.DifferentRepresentations.size(); CloudDataIndex++) {
		if (cloud.DifferentRepresentations[CloudDataIndex].RepresentationIndex == 0) {
			kdtree = (kdt::KDTree<MyPoint>*)cloud.DifferentRepresentations[CloudDataIndex].DATA_ptr;
		}
	}
	if (!kdtree) {
		LOG_CRASHING("KDTree is not built, so search has failed!");
	}
	MyPoint querypoint;
	querypoint[0] = Point.x;
	querypoint[1] = Point.y;
	querypoint[2] = Point.z;
	vector<int> indices = kdtree->knnSearch(querypoint, numberofneighbors);
	for (unsigned int index_i = 0; index_i < indices.size(); index_i++) {
		if (indices[index_i] < 0) {
			LOG_CRASHING("KDTree search has failed, returned index is negative!");
		}
	}
	if (numberofneighbors != indices.size()) {
		LOG_CRASHING("Number of neighbors wanted doesn't match number of neighbors found!");
	}
	for (unsigned int i = 0; i < indices.size(); i++) {
		final_positions[i] = indices[i];
	}


	return final_positions;
}

vector<vec3> TuranEditor::Algorithms::Compute_PCA(const vector<vec3>& points) {
	vector<vec3> PCA_Vectors(3);
	vec3 CenterOfMass(0.0f, 0.0f, 0.0f);
	for (unsigned int PointIndex = 0; PointIndex < points.size(); PointIndex++) {
		CenterOfMass += points[PointIndex];
	}
	CenterOfMass /= points.size();
	vector<vec3> Originated_toCenterOfMass;
	Originated_toCenterOfMass.resize(points.size());
	for (unsigned int PointIndex = 0; PointIndex < points.size(); PointIndex++) {
		Originated_toCenterOfMass[PointIndex] = points[PointIndex] - CenterOfMass;
	}
	Eigen::MatrixXf Y(3, points.size());
	for (unsigned int i = 0; i < points.size(); i++) {
		Y(0, i) = points[i].x;
		Y(1, i) = points[i].y;
		Y(2, i) = points[i].z;
	}
	Eigen::MatrixXf CovarianceMatrix = Y * Y.transpose();
	Eigen::ComplexEigenSolver<Eigen::MatrixXf> solver(CovarianceMatrix, true);
	Eigen::MatrixXcf EigenVectorsMatrix = solver.eigenvectors();
	for (unsigned char vectorindex = 0; vectorindex < 3; vectorindex++) {
		PCA_Vectors[vectorindex].x = EigenVectorsMatrix(0, vectorindex).real();
		PCA_Vectors[vectorindex].y = EigenVectorsMatrix(1, vectorindex).real();
		PCA_Vectors[vectorindex].z = EigenVectorsMatrix(2, vectorindex).real();
	}
	Eigen::VectorXcf EigenVectorsValues = solver.eigenvalues();
	vec3 EigenValuesFinal;
	for (unsigned char EigenValueIndex = 0; EigenValueIndex < 3; EigenValueIndex++) {
		EigenValuesFinal[EigenValueIndex] = EigenVectorsValues(EigenValueIndex).real();
	}
	//Sort
	bool needsSort = true;
	while (needsSort) {
		needsSort = false;
		for (unsigned int EigenValueIndex = 0; EigenValueIndex < 2; EigenValueIndex++) {
			if (abs(EigenValuesFinal[EigenValueIndex]) < abs(EigenValuesFinal[EigenValueIndex + 1])) {
				vec3 Backup_EigenVector = PCA_Vectors[EigenValueIndex];
				PCA_Vectors[EigenValueIndex] = PCA_Vectors[EigenValueIndex + 1];
				PCA_Vectors[EigenValueIndex + 1] = Backup_EigenVector;
				float backup_eigenValue = EigenValuesFinal[EigenValueIndex];
				EigenValuesFinal[EigenValueIndex] = EigenValuesFinal[EigenValueIndex + 1];
				EigenValuesFinal[EigenValueIndex + 1] = backup_eigenValue;
				needsSort = true;
			}
		}
	}
	return PCA_Vectors;
}
vector<vec3> TuranEditor::Algorithms::ProjectPoints_OnPlane_thenTriangulate(const vector<vec3>& points, vec3 Tangent, vec3 Bitangent, vec3 Normal, vector<Triangle>* Faces, vector<Edge>* Edges) {
	vector<vec3> Triangulation;

	//Calculate center of mass of points
	vec3 CenterOfMass(0.0f);
	{
		for (unsigned char PointIndex = 0; PointIndex < points.size(); PointIndex++) {
			CenterOfMass += points[PointIndex];
		}
		CenterOfMass /= 3;
	} 

	//Normalize inputs
	Normal = normalize(Normal);
	Tangent = normalize(Tangent);
	Bitangent = normalize(Bitangent);

	//Project points on tangent plane
	vector<vec2> Points_onPlane(points.size());
	for (unsigned char PointIndex = 0; PointIndex < points.size(); PointIndex++) {
		vec3 Center_toPoint = points[PointIndex] - CenterOfMass;

		float dist_fromsurface = length(Center_toPoint) * dot(normalize(Center_toPoint), Normal);
		vec3 projected_3d = Center_toPoint - Normal * dist_fromsurface;

		vec2& final_2d = Points_onPlane[PointIndex];
		final_2d.x = length(projected_3d) * dot(normalize(projected_3d), Tangent);
		final_2d.y = length(projected_3d) * dot(normalize(projected_3d), Bitangent);
	}
	for (unsigned int MainSearchIndex = 0; MainSearchIndex < Points_onPlane.size(); MainSearchIndex++) {
		for (unsigned int SecondSearchIndex = MainSearchIndex + 1; SecondSearchIndex < Points_onPlane.size(); SecondSearchIndex++) {
			if (Points_onPlane[MainSearchIndex] == Points_onPlane[SecondSearchIndex]) {
				LOG_STATUS("Points are the same!");
				Points_onPlane.erase(Points_onPlane.begin() + SecondSearchIndex);
				SecondSearchIndex--;
			}
		}
	}
	if (Points_onPlane.size() < 3) {
		LOG_STATUS("Some unnecessary points are eliminated and remaining points are less than 3!");
		return Triangulation;
	}


	//Get Triangulation Info
	{
		vector<double> double_points(Points_onPlane.size() * 2);
		for (unsigned int i = 0; i < Points_onPlane.size(); i++) {
			double_points[i * 2] = Points_onPlane[i].x;
			double_points[i * 2 + 1] = Points_onPlane[i].y;
		}
		delaunator::Delaunator d(double_points);
		Triangulation.resize(d.triangles.size());
		for (unsigned int triangle_index = 0; triangle_index < d.triangles.size(); triangle_index += 3) {
			vec3 vertexes[3];
			unsigned int vertexindexes[3]{ UINT32_MAX };

			//Bring point back to 3D
			//And find vertex index to calculate edge info
			for (unsigned int v_index = 0; v_index < 3; v_index++) {
				vec2 v0_in2d = vec2(d.coords[2 * d.triangles[triangle_index + v_index]], d.coords[2 * d.triangles[triangle_index + v_index] + 1]);
				vertexes[v_index] = (v0_in2d.x * Tangent) + v0_in2d.y * Bitangent + CenterOfMass;


				if (Faces) {
					bool isFound = false;
					for (unsigned int searchindex = 0; searchindex < double_points.size() / 2; searchindex++) {
						if (v0_in2d == vec2(double_points[2 * searchindex], double_points[2 * searchindex + 1])) {
							vertexindexes[v_index] = searchindex;
							isFound = true;
							break;
						}
					}
					if (!isFound) {
						LOG_CRASHING("Couldn't find the vertex index!");
					}
				}
			}

			//Save 3D positions
			Triangulation[triangle_index] = vertexes[0];
			Triangulation[triangle_index + 1] = vertexes[1];
			Triangulation[triangle_index + 2] = vertexes[2];

			//Calculate edge info
			if (Faces) {
				Triangle tri;
				tri.VertexIDs[0] = vertexindexes[0];
				tri.VertexIDs[1] = vertexindexes[1];
				tri.VertexIDs[2] = vertexindexes[2];

				//If user wants edge info too
				if (Edges) {
					unsigned int TrisNextEdgeIndex = 0;

					unsigned int SearchVertexIndex0 = vertexindexes[0];
					unsigned int SearchVertexIndex1 = vertexindexes[1];
					bool isEdgeFound = false;
					for (unsigned int EdgeIndex = 0; EdgeIndex < Edges->size(); EdgeIndex++) {
						Edge& e = (*Edges)[EdgeIndex];
						if ((e.VertexIDs[0] == SearchVertexIndex0 && e.VertexIDs[1] == SearchVertexIndex1) ||
							(e.VertexIDs[1] == SearchVertexIndex0 && e.VertexIDs[0] == SearchVertexIndex1)) {
							isEdgeFound = true;
							if (e.TriangleIDs[1] != UINT32_MAX) {
								LOG_CRASHING("An edge connects 2+ faces, this is not possible!");
							}
							e.TriangleIDs[1] = triangle_index / 3;
							tri.EdgeIDs[TrisNextEdgeIndex] = EdgeIndex;
							TrisNextEdgeIndex++;
							break;
						}
					}
					if (!isEdgeFound) {
						Edge newedge;
						newedge.TriangleIDs[0] = triangle_index / 3;
						newedge.VertexIDs[0] = SearchVertexIndex0;
						newedge.VertexIDs[1] = SearchVertexIndex1;
						Edges->push_back(newedge);
						tri.EdgeIDs[TrisNextEdgeIndex] = Edges->size() - 1;
						TrisNextEdgeIndex++;
					}

					SearchVertexIndex0 = vertexindexes[1];
					SearchVertexIndex1 = vertexindexes[2];
					isEdgeFound = false;
					for (unsigned int EdgeIndex = 0; EdgeIndex < (*Edges).size(); EdgeIndex++) {
						Edge& e = (*Edges)[EdgeIndex];
						if ((e.VertexIDs[0] == SearchVertexIndex0 && e.VertexIDs[1] == SearchVertexIndex1) ||
							(e.VertexIDs[1] == SearchVertexIndex0 && e.VertexIDs[0] == SearchVertexIndex1)) {
							isEdgeFound = true;
							if (e.TriangleIDs[1] != UINT32_MAX) {
								LOG_CRASHING("An edge connects 2+ faces, this is not possible!");
							}
							e.TriangleIDs[1] = triangle_index / 3;
							tri.EdgeIDs[TrisNextEdgeIndex] = EdgeIndex;
							TrisNextEdgeIndex++;
							break;
						}
					}
					if (!isEdgeFound) {
						Edge newedge;
						newedge.TriangleIDs[0] = triangle_index / 3;
						newedge.VertexIDs[0] = SearchVertexIndex0;
						newedge.VertexIDs[1] = SearchVertexIndex1;
						(*Edges).push_back(newedge);
						tri.EdgeIDs[TrisNextEdgeIndex] = Edges->size() - 1;
						TrisNextEdgeIndex++;
					}

					SearchVertexIndex0 = vertexindexes[0];
					SearchVertexIndex1 = vertexindexes[2];
					isEdgeFound = false;
					for (unsigned int EdgeIndex = 0; EdgeIndex < (*Edges).size(); EdgeIndex++) {
						Edge& e = (*Edges)[EdgeIndex];
						if ((e.VertexIDs[0] == SearchVertexIndex0 && e.VertexIDs[1] == SearchVertexIndex1) ||
							(e.VertexIDs[1] == SearchVertexIndex0 && e.VertexIDs[0] == SearchVertexIndex1)) {
							isEdgeFound = true;
							if (e.TriangleIDs[1] != UINT32_MAX) {
								LOG_CRASHING("An edge connects 2+ faces, this is not possible!");
							}
							e.TriangleIDs[1] = triangle_index / 3;
							tri.EdgeIDs[TrisNextEdgeIndex] = EdgeIndex;
							TrisNextEdgeIndex++;
							break;
						}
					}
					if (!isEdgeFound) {
						Edge newedge;
						newedge.TriangleIDs[0] = triangle_index / 3;
						newedge.VertexIDs[0] = SearchVertexIndex0;
						newedge.VertexIDs[1] = SearchVertexIndex1;
						(*Edges).push_back(newedge);
						tri.EdgeIDs[TrisNextEdgeIndex] = Edges->size() - 1;
						TrisNextEdgeIndex++;
					}
				}


				Faces->push_back(tri);
			}

		}
	}

	return Triangulation;
}
bool TuranEditor::Algorithms::rayTriangleIntersect(const vec3& orig, const vec3& dir, const vec3& v0, const vec3& v1, const vec3& v2, float& t)
{
	// compute plane's normal
	vec3 v0v1 = v1 - v0;
	vec3 v0v2 = v2 - v0;
	// no need to normalize
	vec3 N = cross(v0v1, v0v2); // N 
	float area2 = N.length();

	// Step 1: finding P

	// check if ray and plane are parallel ?
	float NdotRayDirection = dot(N, dir);
	if (fabs(NdotRayDirection) < FLT_EPSILON) // almost 0 
		return false; // they are parallel so they don't intersect ! 

	// compute d parameter using equation 2
	float d = dot(N, v0);

	// compute t (equation 3)
	t = (dot(N, orig) + d) / NdotRayDirection;
	// check if the triangle is in behind the ray
	if (t < 0) return false; // the triangle is behind 

	// compute the intersection point using equation 1
	vec3 P = orig + t * dir;

	// Step 2: inside-outside test
	vec3 C; // vector perpendicular to triangle's plane 

	// edge 0
	vec3 edge0 = v1 - v0;
	vec3 vp0 = P - v0;
	C = cross(edge0, vp0);
	if (dot(N, C) < 0) return false; // P is on the right side 

	// edge 1
	vec3 edge1 = v2 - v1;
	vec3 vp1 = P - v1;
	C = cross(edge1, vp1);
	if (dot(N, C) < 0)  return false; // P is on the right side 

	// edge 2
	vec3 edge2 = v0 - v2;
	vec3 vp2 = P - v2;
	C = cross(edge2, vp2);
	if (dot(N, C) < 0) return false; // P is on the right side; 

	return true; // this ray hits the triangle 
}

struct FlipVertex {
	unsigned int PointIndex = UINT32_MAX;
	bool isFlipped = false;
};
struct ReimannianEdge {
	unsigned int Node0Index = UINT32_MAX, Node1Index = UINT32_MAX, EdgeIndex = UINT32_MAX;
};
struct ReimannianNode {
	unsigned int NodeIndex = UINT32_MAX;
	vector<unsigned int> KNN;
	vector<unsigned int> EdgeIndexes;
};
struct ReimannianGraph {
	vector<ReimannianNode> Nodes;
	vector<ReimannianEdge> Edges;
	bool isThereAnySameEdge(unsigned int NodeIndex0, unsigned int NodeIndex1) {
		for (unsigned int EdgeIndex = 0; EdgeIndex < Nodes[NodeIndex0].EdgeIndexes.size(); EdgeIndex++) {
			ReimannianEdge& edge = Edges[Nodes[NodeIndex0].EdgeIndexes[EdgeIndex]];
			if ((edge.Node0Index == NodeIndex0 && edge.Node1Index == NodeIndex1) ||
				(edge.Node1Index == NodeIndex0 && edge.Node0Index == NodeIndex1)) {
				return true;
			}
		}
		for (unsigned int EdgeIndex = 0; EdgeIndex < Nodes[NodeIndex1].EdgeIndexes.size(); EdgeIndex++) {
			ReimannianEdge& edge = Edges[Nodes[NodeIndex1].EdgeIndexes[EdgeIndex]];
			if ((edge.Node0Index == NodeIndex0 && edge.Node1Index == NodeIndex1) ||
				(edge.Node1Index == NodeIndex0 && edge.Node0Index == NodeIndex1)) {
				LOG_CRASHING("Edge isn't attached to one of the nodes!");
				return true;
			}
		}
		return false;
	}
};
bool ConstructReimannianGraph(const PointCloud& cloud, unsigned char KNN, ReimannianGraph& OutputGraph) {
	OutputGraph.Edges.clear();	OutputGraph.Nodes.clear();

	//Find KD-Tree structure!
	void* KDTree = nullptr;
	for (unsigned int i = 0; i < cloud.DifferentRepresentations.size(); i++) {
		if (cloud.DifferentRepresentations[i].RepresentationIndex == 0) {
			KDTree = cloud.DifferentRepresentations[i].DATA_ptr;
		}
	}
	if (!KDTree) {
		LOG_CRASHING("Please generate a KDTree before calling HoughBasedNormalReconstruction()!");
	}

	//Create a node for each point in the cloud
	for (unsigned int PointIndex = 0; PointIndex < cloud.PointCount; PointIndex++) {
		OutputGraph.Nodes.push_back(ReimannianNode());
		ReimannianNode& node = OutputGraph.Nodes[OutputGraph.Nodes.size() - 1];

		node.NodeIndex = PointIndex;
		node.KNN = TuranEditor::Algorithms::Searchfor_ClosestNeighbors(cloud, cloud.PointPositions[PointIndex], KNN);
	}

	//Create edges
	for (unsigned int MainNodeIndex = 0; MainNodeIndex < OutputGraph.Nodes.size(); MainNodeIndex++) {
		ReimannianNode& MainNode = OutputGraph.Nodes[MainNodeIndex];
		for (unsigned char KNNIndex = 0; KNNIndex < KNN; KNNIndex++) {
			ReimannianNode& KNNNode = OutputGraph.Nodes[MainNode.KNN[KNNIndex]];

			for (unsigned char NeighborKNNSearchIndex = 0; NeighborKNNSearchIndex < KNN; NeighborKNNSearchIndex++) {
				if (KNNNode.KNN[NeighborKNNSearchIndex] == MainNodeIndex && !OutputGraph.isThereAnySameEdge(MainNode.NodeIndex, KNNNode.NodeIndex)) {
					OutputGraph.Edges.push_back(ReimannianEdge());
					ReimannianEdge& edge = OutputGraph.Edges[OutputGraph.Edges.size() - 1];

					edge.EdgeIndex = OutputGraph.Edges.size() - 1;
					edge.Node0Index = MainNode.NodeIndex;
					edge.Node1Index = KNNNode.NodeIndex;


					MainNode.EdgeIndexes.push_back(edge.EdgeIndex);
					KNNNode.EdgeIndexes.push_back(edge.EdgeIndex);
				}
			}
		}

		if (MainNode.EdgeIndexes.size() == 0) {
			OutputGraph.Edges.clear();
			OutputGraph.Nodes.clear();
			LOG_ERROR("KNN is wrong for Reimannian Graph!");
			return false;
		}
	}

	LOG_STATUS("Reimannian Graph is constructed!");
	return true;
}
struct MinimumSpanningTreeNode {
	vector<unsigned int> NeighborNodeIndexes;
	unsigned int NodeIndex = UINT32_MAX;
};
struct MinimumSpanningTree {
	vector<MinimumSpanningTreeNode> Nodes;
	unsigned int* id;
	MinimumSpanningTree(unsigned int idlistsize) {
		id = new unsigned int[idlistsize] {UINT32_MAX};
		for (unsigned int id_i = 0; id_i < idlistsize; id_i++) {
			id[id_i] = id_i;
		}
	}
	void ConstructTree(const vector<ReimannianEdge>& EdgeList, const vector<vec3>& Normals) {
		unsigned int x, y;
		float weight;
		for (int i = 0; i < EdgeList.size(); ++i)
		{
			x = EdgeList[i].Node0Index;
			y = EdgeList[i].Node1Index;

			vec3 Node0_N = Normals[x];
			vec3 Node1_N = Normals[y];

			weight = 1.0f - dot(Node0_N, Node1_N);

			// Check if the selected edge is creating a cycle or not
			if (root(x) != root(y))
			{
				Nodes[x].NeighborNodeIndexes.push_back(y);
				Nodes[y].NeighborNodeIndexes.push_back(x);

				union1(x, y);
			}
		}
	}
private:
	int root(int x) {
		while (id[x] != x)
		{
			id[x] = id[id[x]];
			x = id[x];
		}
		return x;
	}
	void union1(int x, int y)
	{
		int p = root(x);
		int q = root(y);
		id[p] = id[q];
	}

};
vector<vec3> TuranEditor::Algorithms::HoughBasedNormalReconstruction(const PointCloud& cloud, bool flip_normals, unsigned int K, unsigned int T, unsigned int n_phi, unsigned int n_rot, bool ua, float tol_angle_rad, unsigned int k_density) {
	LOG_STATUS("Normal estimation based on Hough Transform has started!");
	vector<vec3> Normals(cloud.PointCount);
	Eigen::MatrixX3d pc, normals;
	Eigen_Normal_Estimator ne(pc, normals);
	ne.loadPositions(cloud.PointPositions, cloud.PointCount);
	ne.set_K(K);
	ne.set_T(T);
	ne.set_density_sensitive(ua);
	ne.set_n_phi(n_phi);
	ne.set_n_rot(n_rot);
	ne.set_tol_angle_rad(tol_angle_rad);
	ne.set_K_density(k_density);
	ne.estimate_normals();
	ne.getNormalList(Normals);


	vector<FlipVertex> FlipNormalList(Normals.size());
	//Normalize all normals for dot product operations
	for (unsigned int i = 0; i < Normals.size(); i++) {
		Normals[i] = normalize(Normals[i]);
		FlipNormalList[i].PointIndex = i;
		FlipNormalList[i].isFlipped = false;
	}
	if (flip_normals) {
		Normals[0] = -Normals[0];
	}
	FlipNormalList[0].isFlipped = true;

	void* KDTree = nullptr;
	for (unsigned int i = 0; i < cloud.DifferentRepresentations.size(); i++) {
		if (cloud.DifferentRepresentations[i].RepresentationIndex == 0) {
			KDTree = cloud.DifferentRepresentations[i].DATA_ptr;
		}
	}
	if (!KDTree) {
		LOG_CRASHING("Please generate a KDTree before calling HoughBasedNormalReconstruction()!");
	}

	LOG_STATUS("Started to flip normals!");
	ReimannianGraph RGraph;
	unsigned char KNeighbor_forFlipSearch = 7;
	//Construct Reimannian Graph
	while (true) {
		if (ConstructReimannianGraph(cloud, KNeighbor_forFlipSearch, RGraph)) {
			break;
		}
		else {
			KNeighbor_forFlipSearch++;
		}
	}

	//Basic propagation using Reimannian Edges
	/*
	bool isthereUnflipped = true;
	while (isthereUnflipped) {
		isthereUnflipped = false;

		bool isAnyFlipped = false;
		//Find a flipped normal, search for its neighbors, flip if dot isn't in [0,1), mark them as flipped, continue to loop
		for (unsigned int FlipSearchIndex = 0; FlipSearchIndex < FlipNormalList.size(); FlipSearchIndex++) {
			if (FlipNormalList[FlipSearchIndex].isFlipped) {
				continue;
			}

			FlipVertex& MainPoint = FlipNormalList[FlipSearchIndex];
			vec3& MainNormal = Normals[MainPoint.PointIndex];
			const ReimannianNode& ReimanNode = RGraph.Nodes[FlipSearchIndex];


			const vector<unsigned int>& Edges = ReimanNode.EdgeIndexes;
			for (unsigned int NeighborSearchIndex = 0; NeighborSearchIndex < Edges.size(); NeighborSearchIndex++) {
				//Access neighbor's all data and if it isn't processed by the algorithm, skip it
				ReimannianEdge& ReimanEdge = RGraph.Edges[Edges[NeighborSearchIndex]];
				unsigned int NeighborPointIndex = (ReimanEdge.Node0Index == ReimanNode.NodeIndex) ? ReimanEdge.Node1Index : ReimanEdge.Node0Index;
				vec3 NeighborNormal = Normals[NeighborPointIndex];
				if (!FlipNormalList[NeighborPointIndex].isFlipped) {
					continue;
				}

				//If dot product between any processed neighbor is lower than zero, flip the normal
				MainPoint.isFlipped = true;
				isAnyFlipped = true;
				float weight = dot(MainNormal, NeighborNormal);
				if (weight < 0.0f) {
					MainNormal = -MainNormal;
					break;
				}
			}
		}


		for (unsigned int UnflippedPointSearchIndex = 0; UnflippedPointSearchIndex < FlipNormalList.size(); UnflippedPointSearchIndex++) {
			if (!FlipNormalList[UnflippedPointSearchIndex].isFlipped) {
				isthereUnflipped = true;
				break;
			}
		}
		if (isthereUnflipped && !isAnyFlipped) {
			LOG_CRASHING("This shouldn't happen!");
			break;
		}
	}*/


	//Sorted and Weighted Edges
	vector<ReimannianEdge> WeightedReimannianEdges(RGraph.Edges.size());
	float lastMINweight = FLT_MIN;
	for (unsigned int MainSortIndex = 0; MainSortIndex < WeightedReimannianEdges.size(); MainSortIndex++) {
		float currentMINweight = FLT_MAX;
		unsigned int currentMINweightedEdgeIndex = 0;
		for (unsigned int EdgeIndex = MainSortIndex; EdgeIndex < RGraph.Edges.size(); EdgeIndex++) {
			vec3 Node0_N = Normals[RGraph.Edges[EdgeIndex].Node0Index];
			vec3 Node1_N = Normals[RGraph.Edges[EdgeIndex].Node1Index];

			float weight = 1.0f - dot(Node0_N, Node1_N);

			if (weight < currentMINweight && weight > lastMINweight && currentMINweightedEdgeIndex != EdgeIndex) {
				currentMINweight = weight;
				currentMINweightedEdgeIndex = EdgeIndex;
			}
		}
		WeightedReimannianEdges[MainSortIndex] = RGraph.Edges[currentMINweightedEdgeIndex];
		lastMINweight = currentMINweight;
	}

	MinimumSpanningTree MST(1000 * 1000);
	//Fill node list
	for (unsigned int PointIndex = 0; PointIndex < Normals.size(); PointIndex++) {
		MinimumSpanningTreeNode node;
		node.NodeIndex = PointIndex;
		node.NeighborNodeIndexes.clear();
		MST.Nodes.push_back(node);
	}
	MST.ConstructTree(WeightedReimannianEdges, Normals);

	for (unsigned int NodeIndex = 0; NodeIndex < MST.Nodes.size(); NodeIndex++) {
		MinimumSpanningTreeNode& MainNode = MST.Nodes[NodeIndex];
		vec3& MainNormal = Normals[MainNode.NodeIndex];
		::std::cout << "Neighbor Count: " << MainNode.NeighborNodeIndexes.size();
		for (unsigned int NeighborIndex = 0; NeighborIndex < MainNode.NeighborNodeIndexes.size(); NeighborIndex++) {
			vec3 NeighborNormal = Normals[MainNode.NeighborNodeIndexes[NeighborIndex]];
			if (dot(NeighborNormal, MainNormal) < 0) {
				MainNormal = -MainNormal;
				break;
			}
		}
	}
	if (flip_normals) {
		for (unsigned int NormalIndex = 0; NormalIndex < Normals.size(); NormalIndex++) {
			Normals[NormalIndex] = -Normals[NormalIndex];
		}
	}

	return Normals;
}






void Reconstruction_Type0(PointCloud* CLOUD, unsigned char KNearestNeighor, vector<vec3>& TriangulatedPositionsOutput, vector<vec3>& TriangulatedNormalsOutput) {
	unsigned int LastUsedIndex = 0;
	for (unsigned int PointIndex = 0; PointIndex < CLOUD->PointCount; PointIndex++) {
		vector<unsigned int> PointList = TuranEditor::Algorithms::Searchfor_ClosestNeighbors(*CLOUD, CLOUD->PointPositions[PointIndex], KNearestNeighor);

		vector<vec3> PointPositions(PointList.size());
		for (unsigned int i = 0; i < PointPositions.size(); i++) {
			PointPositions[i] = CLOUD->PointPositions[PointList[i]];
		}
		vector<vec3> PCA = TuranEditor::Algorithms::Compute_PCA(PointPositions);
		vector<vec3> ProjectedPoints = TuranEditor::Algorithms::ProjectPoints_OnPlane_thenTriangulate(PointPositions, PCA[0], PCA[1], PCA[2]);

		for (unsigned int TriangulatedPointIndex = 0; TriangulatedPointIndex < ProjectedPoints.size(); TriangulatedPointIndex++) {
			TriangulatedPositionsOutput[LastUsedIndex] = ProjectedPoints[TriangulatedPointIndex];
			TriangulatedNormalsOutput[LastUsedIndex] = -PCA[2];
			LastUsedIndex++;
		}
	}
}

//This algorithm only searches all vertexes as following:
//1) If the vertex is marked as inner in a previous triangulation, skip this vertex
//2) If the vertex is not skipped yet, find all nearest neighbors according to argument (store their VertexIDs)
//3) Search all neighbors if they are marked as inner in a previous triangulation, remove marked ones from the neighbor list
//4) Compute PCA and all other things on the remaining neighbors
void Reconstruction_Type1(PointCloud* CLOUD, unsigned char KNearestNeighor, vector<vec3>& TriangulatedPositionsOutput, vector<vec3>& TriangulatedNormalsOutput) {
	unsigned int LastUsedIndex = 0, PointListBecomesLowerThan3 = 0;
	unsigned char* TriangulationStatusData = nullptr;
	//Find if necessary data is already allocated
	for (unsigned int RepresentationElement = 0; RepresentationElement < CLOUD->DifferentRepresentations.size(); RepresentationElement++) {
		if (CLOUD->DifferentRepresentations[RepresentationElement].RepresentationIndex == 1) {
			TriangulationStatusData = (unsigned char*)CLOUD->DifferentRepresentations[RepresentationElement].DATA_ptr;
			//Zero initialize the previously allocated data
			for (unsigned int PointIndex = 0; PointIndex < CLOUD->PointCount; PointIndex++) {
				TriangulationStatusData[PointIndex] = 0;
			}
			break;
		}
	}
	//If Reconstruction Type1 isn't used before, allocate necessary data
	if (!TriangulationStatusData) {
		CloudRepresentation rep;
		rep.DATA_ptr = new unsigned char[CLOUD->PointCount];
		rep.RepresentationIndex = 1;
		CLOUD->DifferentRepresentations.push_back(rep);
		TriangulationStatusData = (unsigned char*)rep.DATA_ptr;
		//Zero initialize
		for (unsigned int PointIndex = 0; PointIndex < CLOUD->PointCount; PointIndex++) {
			TriangulationStatusData[PointIndex] = 0;
		}
	}
	for (unsigned int PointIndex = 0; PointIndex < CLOUD->PointCount; PointIndex++) {
		if (TriangulationStatusData[PointIndex] == 1) {
			//This vertex was a inner vertex in a previous triangulation, so skip this
			continue;
		}
		vector<unsigned int> PointList = TuranEditor::Algorithms::Searchfor_ClosestNeighbors(*CLOUD, CLOUD->PointPositions[PointIndex], KNearestNeighor);

		//Delete inner vertexes from connection list
		for (int NearestPointsSearchIndex = 0; NearestPointsSearchIndex < PointList.size(); NearestPointsSearchIndex++) {
			if (TriangulationStatusData[PointList[NearestPointsSearchIndex]] == 1) {
				PointList.erase(PointList.begin() + NearestPointsSearchIndex);
				NearestPointsSearchIndex--;	//Because search index is int, this works in element 0
			}
		}
		if (PointList.size() < 3) {
			PointListBecomesLowerThan3++;
			continue;
		}


		vector<vec3> PointPositions(PointList.size());
		for (unsigned int i = 0; i < PointPositions.size(); i++) {
			PointPositions[i] = CLOUD->PointPositions[PointList[i]];
		}
		vector<vec3> PCA = TuranEditor::Algorithms::Compute_PCA(PointPositions);
		vector<TuranEditor::Triangle> Faces; vector<TuranEditor::Edge> Edges;
		vector<vec3> ProjectedPoints = TuranEditor::Algorithms::ProjectPoints_OnPlane_thenTriangulate(PointPositions, PCA[0], PCA[1], PCA[2], &Faces, &Edges);

		//Extract inner and outer vertexes from Edge info
		for (unsigned int EdgeIndex = 0; EdgeIndex < Edges.size(); EdgeIndex++) {
			TuranEditor::Edge& e = Edges[EdgeIndex];
			//Outer Edge
			if (e.TriangleIDs[1] == UINT32_MAX) {
				TriangulationStatusData[PointList[e.VertexIDs[0]]] = 1;
				TriangulationStatusData[PointList[e.VertexIDs[1]]] = 1;
			}
			//Inner Edge
			else {
				if (TriangulationStatusData[PointList[e.VertexIDs[0]]] == 0) {
					TriangulationStatusData[PointList[e.VertexIDs[0]]] = 2;
				}
				if (TriangulationStatusData[PointList[e.VertexIDs[1]]] == 0) {
					TriangulationStatusData[PointList[e.VertexIDs[1]]] = 2;
				}
			}
		}

		for (unsigned int TriangulatedPointIndex = 0; TriangulatedPointIndex < ProjectedPoints.size(); TriangulatedPointIndex++) {
			TriangulatedPositionsOutput[LastUsedIndex] = ProjectedPoints[TriangulatedPointIndex];
			TriangulatedNormalsOutput[LastUsedIndex] = -PCA[2];
			LastUsedIndex++;
		}
	}

	/*
	//Normal handling
	LOG_STATUS("Normal handling is started!");
	float progress = 0.0f;
	for (unsigned int NormalIndex = 0; NormalIndex < TriangulatedNormalsOutput.size(); NormalIndex++) {
		bool isAnyIntersectionFound = false;
		while (!isAnyIntersectionFound) {
			unsigned int UnflippedIntersectionCount = 0, FlippedIntersectionCount = 0;
			for (unsigned int TriangleIndex = 0; TriangleIndex < TriangulatedPositionsOutput.size() / 3; TriangleIndex++) {
				//If an intersection is found
				float unused_raylength = 0.0f;
				if (Algorithms::rayTriangleIntersect(TriangulatedPositionsOutput[NormalIndex], TriangulatedNormalsOutput[NormalIndex],
					TriangulatedPositionsOutput[TriangleIndex * 3], TriangulatedPositionsOutput[TriangleIndex * 3 + 1], TriangulatedPositionsOutput[TriangleIndex * 3 + 2] , unused_raylength)) {
					UnflippedIntersectionCount++;
					isAnyIntersectionFound = true;
				}
				//If an intersection is found
				if (Algorithms::rayTriangleIntersect(TriangulatedPositionsOutput[NormalIndex], -TriangulatedNormalsOutput[NormalIndex],
					TriangulatedPositionsOutput[TriangleIndex * 3], TriangulatedPositionsOutput[TriangleIndex * 3 + 1], TriangulatedPositionsOutput[TriangleIndex * 3 + 2], unused_raylength)) {
					FlippedIntersectionCount++;
					isAnyIntersectionFound = true;
				}
			}

			if (!isAnyIntersectionFound) {
				//LOG_ERROR("You need to distribute the normal!");
			}
			else {
				//LOG_ERROR("You don't need to distribute the normal!");
			}

			if (UnflippedIntersectionCount % 2 && FlippedIntersectionCount % 2 == 0) {
				TriangulatedNormalsOutput[NormalIndex] = -TriangulatedNormalsOutput[NormalIndex];
			}
			break;
		}
		progress += 100.0f / TriangulatedNormalsOutput.size();
		if (progress - float(unsigned int(progress)) <= 100.0f / TriangulatedNormalsOutput.size()) {
			LOG_STATUS("Progress: " + to_string(progress));
		}
	}
	LOG_STATUS("Normal handling is finished!");
	*/
}
struct Cloud_ReconstructionType2Data {
	vector<vector<unsigned int>> ConnectionStatus;
};
void Reconstruction_Type2(PointCloud* CLOUD, unsigned char KNearestNeighor, vector<vec3>& TriangulatedPositionsOutput, vector<vec3>& TriangulatedNormalsOutput) {
	Cloud_ReconstructionType2Data* type2data = nullptr;
	for (unsigned int RepresentationElement = 0; RepresentationElement < CLOUD->DifferentRepresentations.size(); RepresentationElement++) {
		if (CLOUD->DifferentRepresentations[RepresentationElement].RepresentationIndex == 2) {
			type2data = (Cloud_ReconstructionType2Data*)CLOUD->DifferentRepresentations[RepresentationElement].DATA_ptr;
			//Clear previously used data
			type2data->ConnectionStatus.clear();
			type2data->ConnectionStatus.resize(CLOUD->PointCount);
			break;
		}
	}
	if (!type2data) {
		type2data = new Cloud_ReconstructionType2Data;
		CloudRepresentation rep;
		rep.DATA_ptr = type2data;
		rep.RepresentationIndex = 2;
		CLOUD->DifferentRepresentations.push_back(rep);
	}
	unsigned int LastUsedIndex = 0;
	for (unsigned int PointIndex = 0; PointIndex < CLOUD->PointCount; PointIndex++) {
		vector<unsigned int> NearestPointsList = TuranEditor::Algorithms::Searchfor_ClosestNeighbors(*CLOUD, CLOUD->PointPositions[PointIndex], KNearestNeighor);
		vector<unsigned int>& ConnectionStatus = type2data->ConnectionStatus[PointIndex];
		for (int NearestPointsSearchIndex = 0; NearestPointsSearchIndex < NearestPointsList.size(); NearestPointsSearchIndex++) {
			for (unsigned int ConnectedPointIndex = 0; ConnectedPointIndex < ConnectionStatus.size(); ConnectedPointIndex++) {

			}
		}


		vector<vec3> PointPositions(NearestPointsList.size());
		for (unsigned int i = 0; i < PointPositions.size(); i++) {
			PointPositions[i] = CLOUD->PointPositions[NearestPointsList[i]];
		}
		vector<vec3> PCA = TuranEditor::Algorithms::Compute_PCA(PointPositions);
		vector<vec3> ProjectedPoints = TuranEditor::Algorithms::ProjectPoints_OnPlane_thenTriangulate(PointPositions, PCA[0], PCA[1], PCA[2]);
		for (unsigned int FaceIndex = 0; FaceIndex < ProjectedPoints.size() / 3; FaceIndex++) {
			vec3 Vertexes[3]{ ProjectedPoints[FaceIndex * 3], ProjectedPoints[FaceIndex * 3 + 1], ProjectedPoints[FaceIndex * 3 + 2] };

		}

		for (unsigned int TriangulatedPointIndex = 0; TriangulatedPointIndex < ProjectedPoints.size(); TriangulatedPointIndex++) {
			TriangulatedPositionsOutput[LastUsedIndex] = ProjectedPoints[TriangulatedPointIndex];
			TriangulatedNormalsOutput[LastUsedIndex] = PCA[2];
			LastUsedIndex++;
		}
	}
}

#include "3rdPartyLibs/tetgen.h"
void Reconstruction_Type3(PointCloud* CLOUD, vector<vec3>& TriangulatedPositionsOutput, vector<vec3>& TriangulatedNormalsOutput) {
	tetgenio in, out;
	LOG_STATUS("Cloud Point Count: " + to_string(CLOUD->PointCount));

	in.pointlist = new double[CLOUD->PointCount * 3];
	in.numberofpoints = CLOUD->PointCount;
	in.mesh_dim = 3;
	in.numberofpointattributes = 0;
	in.firstnumber = 0;
	for (unsigned int i = 0; i < in.numberofpoints; i++) {
		in.pointlist[i * 3] = CLOUD->PointPositions[i].x;
		in.pointlist[(i * 3) + 1] = CLOUD->PointPositions[i].y;
		in.pointlist[(i * 3) + 2] = CLOUD->PointPositions[i].z;
	}

	tetgenbehavior tetbehavior;
	char* COMMANDs = new char[] {""};
	if (!tetbehavior.parse_commandline(COMMANDs)) {
		LOG_CRASHING("Parsing the command line has failed!");
		return;
	}
	tetrahedralize(&tetbehavior, &in, &out);
	

	TriangulatedPositionsOutput.clear();
	TriangulatedNormalsOutput.clear();

	LOG_STATUS("Tetrahedron count: " + to_string(out.numberoftetrahedra));
	LOG_STATUS("Triangle Faces count: " + to_string(out.numberoftrifaces));
	LOG_STATUS("New Point count: " + to_string(out.numberofpoints));
	LOG_STATUS("Corner count: " + to_string(out.numberofcorners));
	TriangulatedPositionsOutput.resize(out.numberoftetrahedra * 12, vec3(0));
	TriangulatedNormalsOutput.resize(TriangulatedPositionsOutput.size(), vec3(0));
	for (unsigned int TetraHedronIndex = 0; TetraHedronIndex < out.numberoftetrahedra; TetraHedronIndex++) {
		vec3 FirstPoint = vec3(float(in.pointlist[out.tetrahedronlist[TetraHedronIndex * 4]]),
			float(in.pointlist[out.tetrahedronlist[TetraHedronIndex * 4] + 1]),
			float(in.pointlist[out.tetrahedronlist[TetraHedronIndex * 4] + 2]));
		vec3 SecondPoint = vec3(float(in.pointlist[out.tetrahedronlist[(TetraHedronIndex * 4) + 1]]),
			float(in.pointlist[out.tetrahedronlist[(TetraHedronIndex * 4) + 1] + 1]),
				float(in.pointlist[out.tetrahedronlist[(TetraHedronIndex * 4) + 1] + 2]));
		vec3 ThirdPoint = vec3(float(in.pointlist[out.tetrahedronlist[(TetraHedronIndex * 4) + 2]]),
			float(in.pointlist[out.tetrahedronlist[(TetraHedronIndex * 4) + 2] + 1]),
				float(in.pointlist[out.tetrahedronlist[(TetraHedronIndex * 4) + 2] + 2]));
		vec3 FourthPoint = vec3(float(in.pointlist[out.tetrahedronlist[(TetraHedronIndex * 4) + 3]]),
			float(in.pointlist[out.tetrahedronlist[(TetraHedronIndex * 4) + 3] + 1]),
				float(in.pointlist[out.tetrahedronlist[(TetraHedronIndex * 4) + 3] + 2]));

		//First face
		TriangulatedPositionsOutput[TetraHedronIndex * 12] = FirstPoint;
		TriangulatedPositionsOutput[(TetraHedronIndex * 12) + 1] = SecondPoint;
		TriangulatedPositionsOutput[(TetraHedronIndex * 12) + 2] = ThirdPoint;
		TriangulatedNormalsOutput[TetraHedronIndex * 12] = vec3(1, 0, 0);
		TriangulatedNormalsOutput[(TetraHedronIndex * 12) + 1] = vec3(0, 1, 0);
		TriangulatedNormalsOutput[(TetraHedronIndex * 12) + 2] = vec3(0, 0, 1);

		TriangulatedPositionsOutput[(TetraHedronIndex * 12) + 3] = FirstPoint;
		TriangulatedPositionsOutput[(TetraHedronIndex * 12) + 4] = SecondPoint;
		TriangulatedPositionsOutput[(TetraHedronIndex * 12) + 5] = FourthPoint;
		TriangulatedNormalsOutput[(TetraHedronIndex * 12) + 3] = vec3(1, 0, 0);
		TriangulatedNormalsOutput[(TetraHedronIndex * 12) + 4] = vec3(0, 1, 0);
		TriangulatedNormalsOutput[(TetraHedronIndex * 12) + 5] = vec3(0, 0, 1);

		TriangulatedPositionsOutput[(TetraHedronIndex * 12) + 6] = FirstPoint;
		TriangulatedPositionsOutput[(TetraHedronIndex * 12) + 7] = ThirdPoint;
		TriangulatedPositionsOutput[(TetraHedronIndex * 12) + 8] = FourthPoint;
		TriangulatedNormalsOutput[(TetraHedronIndex * 12) + 6] = vec3(1, 0, 0);
		TriangulatedNormalsOutput[(TetraHedronIndex * 12) + 7] = vec3(0, 1, 0);
		TriangulatedNormalsOutput[(TetraHedronIndex * 12) + 8] = vec3(0, 0, 1);

		TriangulatedPositionsOutput[(TetraHedronIndex * 12) + 9] = SecondPoint;
		TriangulatedPositionsOutput[(TetraHedronIndex * 12) + 10] = ThirdPoint;
		TriangulatedPositionsOutput[(TetraHedronIndex * 12) + 11] = FourthPoint;
		TriangulatedNormalsOutput[(TetraHedronIndex * 12) + 9] = vec3(1, 0, 0);
		TriangulatedNormalsOutput[(TetraHedronIndex * 12) + 10] = vec3(0, 1, 0);
		TriangulatedNormalsOutput[(TetraHedronIndex * 12) + 11] = vec3(0, 0, 1);
	}
}

void SaveReconstructedSurface(const char* PATH, const vector<vec3>& TriangulatedPositions, const vector<vec3>& TriangulatedNormals, unsigned char KNN, unsigned char Type) {
	flatbuffers::FlatBufferBuilder build(1024);

	if (TriangulatedPositions.size() != TriangulatedNormals.size()) {
		LOG_CRASHING("Normal and position counts are not same!");
	}

	LOG_CRASHING("This function isn't supported!");
	/*
	vector<Vec3f> Positions(TriangulatedPositions.size()), Normals(TriangulatedNormals.size());
	for (unsigned int VertexIndex = 0; VertexIndex < TriangulatedPositions.size(); VertexIndex++) {
		Positions[VertexIndex] = Vec3f(TriangulatedPositions[VertexIndex].x, TriangulatedPositions[VertexIndex].y, TriangulatedPositions[VertexIndex].z);
		Normals[VertexIndex] = Vec3f(TriangulatedNormals[VertexIndex].x, TriangulatedNormals[VertexIndex].y, TriangulatedNormals[VertexIndex].z);
	}
	auto RecMesh = CreateRecMeshDirect(build, &Positions, &Normals, Type, KNN);
	auto CompiledGeodesic = CreateResource(build, Resource_Type_ReconstructedMesh, RecMesh.Union());
	build.Finish(CompiledGeodesic);

	void* data_ptr = build.GetBufferPointer();
	unsigned int data_size = build.GetSize();


	//Check if the data is complete!
	flatbuffers::Verifier verifier((uint8_t*)data_ptr, data_size);
	if (!VerifyResourceBuffer(verifier)) {
		LOG_CRASHING("Verification of the reconstructed mesh has failed!\n");
	}
	TAPIFILESYSTEM::Overwrite_BinaryFile(PATH, data_ptr, data_size);*/
}

unsigned int TuranEditor::Algorithms::ReconstructSurface(PointCloud* CLOUD, unsigned char KNearestNeighor, GFX_API::VertexAttributeLayout& PositionNormal_VertexAttrib, unsigned char Type, const char* SavePath) {
	vector<vec3> TriangulatedCloudPositions(CLOUD->PointCount * KNearestNeighor * 7);
	vector<vec3> TriangulatedCloudNormals(TriangulatedCloudPositions.size());
	Algorithms::Generate_KDTree(*CLOUD);

	switch (Type) {
	case 0:
		Reconstruction_Type0(CLOUD, KNearestNeighor, TriangulatedCloudPositions, TriangulatedCloudNormals);
		break;
	case 1:
		Reconstruction_Type1(CLOUD, KNearestNeighor, TriangulatedCloudPositions, TriangulatedCloudNormals);
		break;
	case 3:
		Reconstruction_Type3(CLOUD, TriangulatedCloudPositions, TriangulatedCloudNormals);
		break;
	default:
		LOG_CRASHING("There is no surface reconstruction that has this TypeIndex!");
		return UINT32_MAX;
	}

	//Upload mesh buffer
	unsigned int MESHBUFFER_ID = UINT32_MAX;
	{
		vector<vec3> TriangulatedFinalDatas(TriangulatedCloudPositions.size() * 2);
		for (unsigned int VertexID = 0; VertexID < TriangulatedCloudPositions.size(); VertexID++) {
			TriangulatedFinalDatas[VertexID] = TriangulatedCloudPositions[VertexID];
			TriangulatedFinalDatas[TriangulatedCloudPositions.size() + VertexID] = TriangulatedCloudNormals[VertexID];
		}
		MESHBUFFER_ID = GFXContentManager->Upload_MeshBuffer(PositionNormal_VertexAttrib, TriangulatedFinalDatas.data(),
			TriangulatedFinalDatas.size() * 12, TriangulatedCloudPositions.size(), nullptr, 0);

	}

	if (SavePath) {
		SaveReconstructedSurface(SavePath, TriangulatedCloudPositions, TriangulatedCloudNormals, KNearestNeighor, Type);
	}
	return MESHBUFFER_ID;
}
unsigned int TuranEditor::Algorithms::LoadReconstructedSurface_fromDisk(const char* PATH, GFX_API::VertexAttributeLayout& PositionNormal_VertexAttrib, unsigned char& KNN, unsigned char& Type) {
	unsigned int datasize = 0;
	void* bdata = TAPIFILESYSTEM::Read_BinaryFile(PATH, datasize);
	if (!bdata) { LOG_CRASHING("Loading has failed!");	return UINT32_MAX; }

	LOG_CRASHING("This function isn't supported!");
	return UINT32_MAX;
	/*
	const Resource* res = GetResource(bdata);
	if (!res) { LOG_CRASHING("File is not a verified resource!");	return UINT32_MAX; }
	const RecMesh* LoadedMesh = res->TYPE_as_ReconstructedMesh();
	if (!LoadedMesh) { LOG_CRASHING("File is not a reconstructed mesh!"); return UINT32_MAX; }

	KNN = LoadedMesh->KNN();
	Type = LoadedMesh->ReconstructionType();
	if (LoadedMesh->Normals()->size() != LoadedMesh->Positions()->size()) { LOG_CRASHING("Loaded mesh's normal and position count doesn't match!"); return UINT32_MAX; };

	vector<vec3> TriangulatedFinalDatas(LoadedMesh->Normals()->size() * 2);
	for (unsigned int PointIndex = 0; PointIndex < LoadedMesh->Positions()->size(); PointIndex++) {
		TriangulatedFinalDatas[PointIndex] = vec3(LoadedMesh->Positions()->Get(PointIndex)->x(), LoadedMesh->Positions()->Get(PointIndex)->y(), LoadedMesh->Positions()->Get(PointIndex)->z());
		TriangulatedFinalDatas[LoadedMesh->Normals()->size() + PointIndex] = vec3(LoadedMesh->Normals()->Get(PointIndex)->x(), LoadedMesh->Normals()->Get(PointIndex)->y(), LoadedMesh->Normals()->Get(PointIndex)->z());
	}
	unsigned int MESHBUFFER_ID = GFXContentManager->Upload_MeshBuffer(PositionNormal_VertexAttrib, TriangulatedFinalDatas.data(),
		TriangulatedFinalDatas.size() * 12, LoadedMesh->Normals()->size(), nullptr, 0);

	return MESHBUFFER_ID;*/
}


void TuranEditor::Algorithms::HoughBasedSphereDetection(PointCloud* CLOUD, double MinSphereRadius, unsigned int BoundingCubeRes, unsigned int MaximaThreshold,std::vector<glm::vec4>& FoundSpheres) {
	FoundSpheres.clear();
	if (CLOUD->PointCount == 0) {
		LOG_CRASHING("Point Cloud is empty!");
	}
	//Find bounding box of the point cloud
	dvec3 BOUNDINGMIN(DBL_MAX), BOUNDINGMAX(-DBL_MAX);
	double MINDIST(DBL_MAX);
	for (unsigned int i = 0; i < CLOUD->PointCount; i++) {
		vec3 Point = CLOUD->PointPositions[i];
		for (unsigned int DistCheckIndex = i + 1; DistCheckIndex < CLOUD->PointCount; DistCheckIndex++) {
			double dist = length(dvec3(Point) - dvec3(CLOUD->PointPositions[DistCheckIndex]));
			if (dist < MINDIST) { MINDIST = dist; }
		}
		if (Point.x < BOUNDINGMIN.x) { BOUNDINGMIN.x = Point.x;}
		if (Point.y < BOUNDINGMIN.y) { BOUNDINGMIN.y = Point.y;}
		if (Point.z < BOUNDINGMIN.z) { BOUNDINGMIN.z = Point.z;}

		if (Point.x > BOUNDINGMAX.x) { BOUNDINGMAX.x = Point.x;}
		if (Point.y > BOUNDINGMAX.y) { BOUNDINGMAX.y = Point.y;}
		if (Point.z > BOUNDINGMAX.z) { BOUNDINGMAX.z = Point.z;}
	}
	dvec3 CUBESIZE = BOUNDINGMAX - BOUNDINGMIN;
	BOUNDINGMIN -= CUBESIZE / 4.0;
	BOUNDINGMAX += CUBESIZE / 4.0;
	CUBESIZE *= 1.5;
	if (double(CUBESIZE.x / double(BoundingCubeRes + 1)) < FLT_EPSILON ||
		double(CUBESIZE.y / double(BoundingCubeRes + 1)) < FLT_EPSILON ||
		double(CUBESIZE.z / double(BoundingCubeRes + 1)) < FLT_EPSILON) {
		LOG_CRASHING("Bounding box of the point cloud so small that float precision isn't precise enough! Hough transform has failed!");
		return;
	}
	dvec3 VOXELDIST = CUBESIZE / dvec3(BoundingCubeRes + 1);
	const double DISCRETIZER = MINDIST;

	double MAXR = sqrt((CUBESIZE.x * CUBESIZE.x) + (CUBESIZE.y * CUBESIZE.y) + (CUBESIZE.z * CUBESIZE.z));
	const unsigned int R_ARRAY_SIZE = MAXR * 1.25 / DISCRETIZER;

	LOG_STATUS("Creating Hough Space...");
	//r->VoxelX->VoxelY->VoxelZ
	//Create and set the size of Hough Space
	std::vector<std::vector<std::vector<std::vector<unsigned int>>>> HoughSpace;
	HoughSpace.resize(R_ARRAY_SIZE);
	for (unsigned int r_index = 0; r_index < R_ARRAY_SIZE; r_index++) {
		HoughSpace[r_index].resize(BoundingCubeRes);
		for (unsigned int voxelindex_x = 0; voxelindex_x < BoundingCubeRes; voxelindex_x++) {
			HoughSpace[r_index][voxelindex_x].resize(BoundingCubeRes);
			for (unsigned int voxelindex_y = 0; voxelindex_y < BoundingCubeRes; voxelindex_y++) {
				HoughSpace[r_index][voxelindex_x][voxelindex_y].resize(BoundingCubeRes, 0);
			}
		}
		
	}



	LOG_STATUS("Filling Hough Space...");
	for (unsigned int i = 0; i < CLOUD->PointCount; i++) {
		vec3 Point = CLOUD->PointPositions[i];
		for (unsigned int VoxelIndexX = 0; VoxelIndexX < BoundingCubeRes; VoxelIndexX++) {
			for (unsigned int VoxelIndexY = 0; VoxelIndexY < BoundingCubeRes; VoxelIndexY++) {
				for (unsigned int VoxelIndexZ = 0; VoxelIndexZ < BoundingCubeRes; VoxelIndexZ++) {
					dvec3 VoxelLoc = dvec3(BOUNDINGMIN.x + (VOXELDIST.x / 2) + (VOXELDIST.x * VoxelIndexX), 
						BOUNDINGMIN.y + (VOXELDIST.y / 2) + (VOXELDIST.y * VoxelIndexY), 
						BOUNDINGMIN.z + (VOXELDIST.z / 2) + (VOXELDIST.z * VoxelIndexZ));
					double R = glm::length(dvec3(Point) - VoxelLoc);
					if (R > MinSphereRadius) {
						unsigned int DiscretizedR = R / DISCRETIZER;
						HoughSpace[DiscretizedR][VoxelIndexX][VoxelIndexY][VoxelIndexZ]++;
					}
				}
			}
		}
	}

	LOG_STATUS("Searching local maximas...");
	for (int rindex_i = 0; rindex_i < R_ARRAY_SIZE; rindex_i++) {
		if (rindex_i * DISCRETIZER < MinSphereRadius) {
			continue;
		}
		for (int VoxelX_i = 0; VoxelX_i < BoundingCubeRes; VoxelX_i++) {
			for (int VoxelY_i = 0; VoxelY_i < BoundingCubeRes; VoxelY_i++) {
				for (int VoxelZ_i = 0; VoxelZ_i < BoundingCubeRes; VoxelZ_i++) {
					unsigned int& Maxima = HoughSpace[rindex_i][VoxelX_i][VoxelY_i][VoxelZ_i];

					if (Maxima < MaximaThreshold) {
						continue;
					}
					//Is that local maxima? (11x5x5x5)
					bool isFoundNewMaxima = false;
					for (int roffset = -5; roffset <= 5 && !isFoundNewMaxima; roffset++) {
						for (int xoffset = -6; xoffset <= 6 && !isFoundNewMaxima; xoffset++) {
							for (int yoffset = -6; yoffset <= 6 && !isFoundNewMaxima; yoffset++) {
								for (int zoffset = -6; zoffset <= 6 && !isFoundNewMaxima; zoffset++) {
									if (rindex_i + roffset >= R_ARRAY_SIZE || rindex_i + roffset < 0 ||
										VoxelX_i + xoffset >= BoundingCubeRes || VoxelX_i + xoffset < 0 ||
										VoxelY_i + yoffset >= BoundingCubeRes || VoxelY_i + yoffset < 0 ||
										VoxelZ_i + zoffset >= BoundingCubeRes || VoxelZ_i + zoffset < 0) {
										continue;
									}
									if (roffset == 0 && xoffset == 0 && yoffset == 0 && zoffset == 0) { continue; }
									unsigned int Check = HoughSpace[rindex_i + roffset]
										[VoxelX_i + xoffset][VoxelY_i + yoffset][VoxelZ_i + zoffset];
									if (Check > Maxima) {
										isFoundNewMaxima = true;
									}
									else if (Check == Maxima) {
										Maxima--;
										isFoundNewMaxima = true;
									}
								}
							}
						}
					}

					//If this is not local maxima, skip it
					if (isFoundNewMaxima) {
						continue;
					}

					dvec4 VoxelLoc = dvec4(BOUNDINGMIN.x + (VOXELDIST.x / 2) + (VOXELDIST.x * VoxelX_i),
						BOUNDINGMIN.y + (VOXELDIST.y / 2) + (VOXELDIST.y * VoxelY_i),
						BOUNDINGMIN.z + (VOXELDIST.z / 2) + (VOXELDIST.z * VoxelZ_i),
						rindex_i * DISCRETIZER);
					FoundSpheres.push_back(VoxelLoc);
				}
			}
		}
	}

}


//If you want to visualize SDF volume, you may want to sample locations. SampleLocations will clear and reallocate the given vector.
//SampleLocations and the returned vector<float> vectors have the same order
vector<float> TuranEditor::Algorithms::SDF_FromPointCloud(PointCloud* CLOUD, uvec3 SDFCubeRes, unsigned int NormalListIndex, float SamplingD, vector<vec3>* SampleLocations) {
	//Create bounding box
	dvec3 BOUNDINGMIN(DBL_MAX), BOUNDINGMAX(-DBL_MAX), CUBESIZE(0.0);
	{
		//Find bounding box of the point cloud
		for (unsigned int i = 0; i < CLOUD->PointCount; i++) {
			vec3 Point = CLOUD->PointPositions[i];
			if (Point.x < BOUNDINGMIN.x) { BOUNDINGMIN.x = Point.x; }
			if (Point.y < BOUNDINGMIN.y) { BOUNDINGMIN.y = Point.y; }
			if (Point.z < BOUNDINGMIN.z) { BOUNDINGMIN.z = Point.z; }

			if (Point.x > BOUNDINGMAX.x) { BOUNDINGMAX.x = Point.x; }
			if (Point.y > BOUNDINGMAX.y) { BOUNDINGMAX.y = Point.y; }
			if (Point.z > BOUNDINGMAX.z) { BOUNDINGMAX.z = Point.z; }
		}
		CUBESIZE = BOUNDINGMAX - BOUNDINGMIN;
		BOUNDINGMIN -= CUBESIZE / 20.0;
		BOUNDINGMAX += CUBESIZE / 20.0;
		CUBESIZE *= 1.1;
	}
	dvec3 SAMPLE_DIST = CUBESIZE / dvec3(SDFCubeRes - uvec3(1));

	vector<vec3>* SAMPLELOCs = nullptr;
	if (!SampleLocations) {
		SAMPLELOCs = new vector<vec3>;
	}
	else {
		SAMPLELOCs = SampleLocations;
	}
	//Create sample points
	{
		SAMPLELOCs->resize(SDFCubeRes.x * SDFCubeRes.y * SDFCubeRes.z);

		for (unsigned int SamplePointIndex = 0; SamplePointIndex < SDFCubeRes.x * SDFCubeRes.y * SDFCubeRes.z; SamplePointIndex++) {
			unsigned int SampleIndex_Z = SamplePointIndex / (SDFCubeRes.x * SDFCubeRes.y);
			unsigned int SampleIndex_Y = (SamplePointIndex - (SampleIndex_Z * SDFCubeRes.x * SDFCubeRes.y)) / SDFCubeRes.x;
			unsigned int SampleIndex_X = SamplePointIndex - (SampleIndex_Z * SDFCubeRes.x * SDFCubeRes.y) - (SampleIndex_Y * SDFCubeRes.x);

			(*SAMPLELOCs)[SamplePointIndex] = vec3(
				BOUNDINGMIN.x + (SampleIndex_X * SAMPLE_DIST.x),
				BOUNDINGMIN.y + (SampleIndex_Y * SAMPLE_DIST.y),
				BOUNDINGMIN.z + (SampleIndex_Z * SAMPLE_DIST.z));
		}
	}

	//For each sample point, find nearest point of the cloud...
	//..Calculate the distance along the three edges (x-y-z edges)...
	//...Keep the lowest distance. If distance is longer than the edge, it is very outside or inside...
	//...(normal based calculation is not important, pca normal is fine)
	//Very inside points are green, very outside points are blue, 0-1 values are black-to-white
	std::vector<float> SDFs;
	LOG_STATUS("Finding SDFs of the samples");
	SDFs.resize(SDFCubeRes.x * SDFCubeRes.y * SDFCubeRes.z, FLT_MAX);
	const float maxsampledist = length(SAMPLE_DIST) * SamplingD / 2;
	for (unsigned int SamplePointIndex = 0; SamplePointIndex < SDFCubeRes.x * SDFCubeRes.y * SDFCubeRes.z; SamplePointIndex++) {
		vec3 SamplePoint = (*SAMPLELOCs)[SamplePointIndex];

		unsigned int ClosestPCPointIndex = TuranEditor::Algorithms::Searchfor_ClosestNeighbors(*CLOUD, SamplePoint, 1)[0];
		vec3 Sample_toPCPoint = SamplePoint - CLOUD->PointPositions[ClosestPCPointIndex];
		float dist_fromsurface = length(Sample_toPCPoint) * dot(normalize(Sample_toPCPoint), CLOUD->PointNormals[NormalListIndex].Normals[ClosestPCPointIndex]);

		if (length(Sample_toPCPoint) > maxsampledist) {
			if (dist_fromsurface < 0.0) {
				SDFs[SamplePointIndex] = -FLT_MAX;
			}
		}
		else {
			SDFs[SamplePointIndex] = dist_fromsurface;
		}
		if (SDFCubeRes.x * SDFCubeRes.y * SDFCubeRes.z > 100 * 100 * 100) {
			if (SamplePointIndex % (SDFCubeRes.x * SDFCubeRes.y * SDFCubeRes.z / 100000) == 0) {
				LOG_STATUS("%" + to_string(float(SamplePointIndex) / (float(SDFCubeRes.x * SDFCubeRes.y * SDFCubeRes.z) / 100.0)));
			}
		}
		else {
			if (SamplePointIndex % (SDFCubeRes.x * SDFCubeRes.y * SDFCubeRes.z / 100) == 0) {
				LOG_STATUS("%" + to_string(SamplePointIndex / (SDFCubeRes.x * SDFCubeRes.y * SDFCubeRes.z / 100)));
			}
		}
	}

	return SDFs;
}

float dot2(vec3 v) { return dot(v, v); }
float udTriangle(vec3 v1, vec3 v2, vec3 v3, vec3 p)
{
	// prepare data    
	vec3 v21 = v2 - v1; vec3 p1 = p - v1;
	vec3 v32 = v3 - v2; vec3 p2 = p - v2;
	vec3 v13 = v1 - v3; vec3 p3 = p - v3;
	vec3 nor = cross(v21, v13);

	return sqrt( // inside/outside test    
		(sign(dot(cross(v21, nor), p1)) +
			sign(dot(cross(v32, nor), p2)) +
			sign(dot(cross(v13, nor), p3)) < 2.0)
		?
		// 3 edges    
		glm::min(glm::min(
			dot2(v21 * glm::clamp(dot(v21, p1) / dot2(v21), 0.0f, 1.0f) - p1),
			dot2(v32 * glm::clamp(dot(v32, p2) / dot2(v32), 0.0f, 1.0f) - p2)),
			dot2(v13 * glm::clamp(dot(v13, p3) / dot2(v13), 0.0f, 1.0f) - p3))
		:
		// 1 face    
		dot(nor, p1) * dot(nor, p1) / dot2(nor));
}
vector<float> TuranEditor::Algorithms::SDF_FromVertexBuffer(const std::vector<vec3>& VertexPositions, const std::vector<vec3> VertexNormals, uvec3 SDFCubeRes, 
	vec3 BOUNDINGMIN, vec3 BOUNDINGMAX, vector<vec3>* SampleLocations, const PointCloud* const RefPC, unsigned int RefPC_NormalIndex, float SamplingD) {
	if ((VertexNormals.size() && VertexPositions.size() != VertexNormals.size()) || VertexPositions.size() % 3) {
		LOG_CRASHING("SDF_FromVertexBuffer doesn't use indexing info, so your mesh buffer should be given in non-indexed format! Also VertexNormals and Positions maybe doesn't match");
		return vector<float>();
	}
	//Create bounding box
	dvec3 CUBESIZE(0.0);
	CUBESIZE = BOUNDINGMAX - BOUNDINGMIN;
	BOUNDINGMIN -= CUBESIZE / 20.0;
	BOUNDINGMAX += CUBESIZE / 20.0;
	CUBESIZE *= 1.1;
	dvec3 SAMPLE_DIST = CUBESIZE / dvec3(SDFCubeRes - uvec3(1));

	vector<vec3>* SAMPLELOCs = nullptr;
	if (!SampleLocations) {
		SAMPLELOCs = new vector<vec3>;
	}
	else {
		SAMPLELOCs = SampleLocations;
	}
	//Create sample points
	{
		SAMPLELOCs->resize(SDFCubeRes.x * SDFCubeRes.y * SDFCubeRes.z);

		for (unsigned int SamplePointIndex = 0; SamplePointIndex < SDFCubeRes.x * SDFCubeRes.y * SDFCubeRes.z; SamplePointIndex++) {
			unsigned int SampleIndex_Z = SamplePointIndex / (SDFCubeRes.x * SDFCubeRes.y);
			unsigned int SampleIndex_Y = (SamplePointIndex - (SampleIndex_Z * SDFCubeRes.x * SDFCubeRes.y)) / SDFCubeRes.x;
			unsigned int SampleIndex_X = SamplePointIndex - (SampleIndex_Z * SDFCubeRes.x * SDFCubeRes.y) - (SampleIndex_Y * SDFCubeRes.x);

			(*SAMPLELOCs)[SamplePointIndex] = vec3(
				BOUNDINGMIN.x + (SampleIndex_X * SAMPLE_DIST.x),
				BOUNDINGMIN.y + (SampleIndex_Y * SAMPLE_DIST.y),
				BOUNDINGMIN.z + (SampleIndex_Z * SAMPLE_DIST.z));
		}
	}

	std::vector<float> SDFs;
	LOG_STATUS("Finding SDFs of the samples");
	SDFs.resize(SDFCubeRes.x * SDFCubeRes.y * SDFCubeRes.z, FLT_MAX);
	for (unsigned int SamplePointIndex = 0; SamplePointIndex < SDFCubeRes.x * SDFCubeRes.y * SDFCubeRes.z; SamplePointIndex++) {
		vec3 SamplePoint = (*SAMPLELOCs)[SamplePointIndex];

		float DIST = FLT_MAX;
		unsigned int nearest_faceindex = 0;
		for (unsigned int FaceID = 0; FaceID < VertexPositions.size() / 3; FaceID++) {
			vec3 v0 = VertexPositions[FaceID * 3], v1 = VertexPositions[(FaceID * 3) + 1], v2 = VertexPositions[(FaceID * 3) + 2];
			float dist_fromsurface = udTriangle(v0, v1, v2, SamplePoint);

			//If distance to triangle is lower, check the distance against point cloud
			if (dist_fromsurface < DIST) {
				DIST = dist_fromsurface; nearest_faceindex = FaceID; 
			}
			else { continue; }

		}
		if (RefPC) {
			if (RefPC_NormalIndex == UINT32_MAX) { LOG_CRASHING("You have to specify the normal index of the reference point cloud!"); continue; }
			else {
				vec3 v0 = VertexPositions[nearest_faceindex * 3], v1 = VertexPositions[(nearest_faceindex * 3) + 1], v2 = VertexPositions[(nearest_faceindex * 3) + 2];
				unsigned int nearest_pointindex = TuranEditor::Algorithms::Searchfor_ClosestNeighbors(*RefPC, (v0 + v1 + v2) / vec3(3.0f), uint(1))[0];
				vec3 nearestpoint_pos = RefPC->PointPositions[nearest_pointindex];
				if (length(nearestpoint_pos - SamplePoint) > SamplingD * length(SAMPLE_DIST) / 2.0) { DIST = FLT_MAX; }
			}
		}
		SDFs[SamplePointIndex] = DIST;

		vec3 v0 = VertexPositions[nearest_faceindex * 3], v1 = VertexPositions[(nearest_faceindex * 3) + 1], v2 = VertexPositions[(nearest_faceindex * 3) + 2];
		vec3 Normal = cross(v2 - v0, v1 - v0);
		//If vertex normals are provided, use vertex normals to fix normal direction
		if (VertexNormals.size()) {
			//Find the dominant direction
			vec3 n0 = VertexNormals[nearest_faceindex * 3], n1 = VertexNormals[(nearest_faceindex * 3) + 1], n2 = VertexNormals[(nearest_faceindex * 3) + 2];
			Normal = normalize(n0 + n1 + n2);
		}
		SDFs[SamplePointIndex] *= sign(dot(normalize(Normal), normalize(SamplePoint - v0)));


		if (SamplePointIndex % (SDFCubeRes.x * SDFCubeRes.y * SDFCubeRes.z / 100) == 0) {
			LOG_STATUS("%" + to_string(float(SamplePointIndex) / (float(SDFCubeRes.x * SDFCubeRes.y * SDFCubeRes.z) / 100.0)));
		}
	}

	return SDFs;
}


#include "Editor/TUBITAK/MarchingCubes_LookUpTable.h"
vector<vec3> TuranEditor::Algorithms::MarchingCubes(uvec3 SDFResolution, const vector<float>& SDFs, const vector<vec3>& SamplePositions, const PointCloud* RefPC, vector<vec3>* OutputNormals) {
	if (SDFs.size() != SamplePositions.size()) {
		LOG_CRASHING("SDFs and SampleLocations isn't matching!");
		return vector<vec3>();
	}
	unsigned int SDFCellCount = (SDFResolution.x - 1) * (SDFResolution.y - 1) * (SDFResolution.z - 1);
	LOG_STATUS("Running Marching Cubes");
	std::vector<vec3> VertexBuffer;
	for (unsigned int CellIndex = 0; CellIndex < SDFCellCount; CellIndex++) {
		static constexpr uvec2 EdgeList[12] = {
			uvec2(0, 1), uvec2(1, 2), uvec2(2,3), uvec2(3,0),
			uvec2(4, 5), uvec2(5,6), uvec2(6,7), uvec2(7,4),
			uvec2(0, 4), uvec2(1,5), uvec2(2,6), uvec2(3,7)
		};
		unsigned int CellIndex_Z = CellIndex / ((SDFResolution.y - 1) * (SDFResolution.x - 1));
		unsigned int CellIndex_Y = (CellIndex - (CellIndex_Z * (SDFResolution.y - 1) * (SDFResolution.x - 1))) / (SDFResolution.x - 1);
		unsigned int CellIndex_X = CellIndex - (CellIndex_Z * (SDFResolution.y - 1) * (SDFResolution.x - 1)) - (CellIndex_Y * (SDFResolution.x - 1));

		unsigned int SampleIndexes[8] = { (CellIndex_Z * SDFResolution.x * SDFResolution.y) + (CellIndex_Y * SDFResolution.x) + CellIndex_X,
		(CellIndex_Z * SDFResolution.x * SDFResolution.y) + (CellIndex_Y * SDFResolution.x) + CellIndex_X + 1,
		((CellIndex_Z + 1) * SDFResolution.x * SDFResolution.y) + (CellIndex_Y * SDFResolution.x) + CellIndex_X + 1,
		((CellIndex_Z + 1) * SDFResolution.x * SDFResolution.y) + (CellIndex_Y * SDFResolution.x) + CellIndex_X,
		(CellIndex_Z * SDFResolution.x * SDFResolution.y) + ((CellIndex_Y + 1) * SDFResolution.x) + CellIndex_X,
		(CellIndex_Z * SDFResolution.x * SDFResolution.y) + ((CellIndex_Y + 1) * SDFResolution.x) + CellIndex_X + 1,
		((CellIndex_Z + 1) * SDFResolution.x * SDFResolution.y) + ((CellIndex_Y + 1) * SDFResolution.x) + CellIndex_X + 1,
		((CellIndex_Z + 1) * SDFResolution.x * SDFResolution.y) + ((CellIndex_Y + 1) * SDFResolution.x) + CellIndex_X
		};
		/*
		unsigned int SampleIndexes[8]{
			(CellIndex_Z * SDFResolution.x * SDFResolution.y) + (CellIndex_Y * SDFResolution.y) + CellIndex_X,
			(CellIndex_Z * SDFResolution.x * SDFResolution.y) + (CellIndex_Y * SDFResolution.y) + CellIndex_X + 1,
			(CellIndex_Z * SDFResolution.x * SDFResolution.y) + ((CellIndex_Y + 1) * SDFResolution.y) + CellIndex_X,
			(CellIndex_Z * SDFResolution.x * SDFResolution.y) + ((CellIndex_Y + 1) * SDFResolution.y) + CellIndex_X + 1,
			((CellIndex_Z + 1) * SDFResolution.x * SDFResolution.y) + (CellIndex_Y * SDFResolution.y) + CellIndex_X,
			((CellIndex_Z + 1) * SDFResolution.x * SDFResolution.y) + (CellIndex_Y * SDFResolution.y) + CellIndex_X + 1,
			((CellIndex_Z + 1) * SDFResolution.x * SDFResolution.y) + ((CellIndex_Y + 1) * SDFResolution.y) + CellIndex_X,
			((CellIndex_Z + 1) * SDFResolution.x * SDFResolution.y) + ((CellIndex_Y + 1) * SDFResolution.y) + CellIndex_X + 1
		};*/
		 
		unsigned char LookUpTable_Index = 0;
		for (unsigned char CornerIndex = 0; CornerIndex < 8; CornerIndex++) {
			if (abs(SDFs[SampleIndexes[CornerIndex]]) != FLT_MAX &&
				SDFs[SampleIndexes[CornerIndex]] < 0.0) { 
				LookUpTable_Index |= 1 << CornerIndex;
			}
		} 

		unsigned char Table_EdgeSearchIndex = 0;  
		unsigned int VertexBufferSize = VertexBuffer.size();
		while (MarchingCubes_LookUpTable[LookUpTable_Index][Table_EdgeSearchIndex] != -1) {
			unsigned char EdgeIndex = MarchingCubes_LookUpTable[LookUpTable_Index][Table_EdgeSearchIndex];
			unsigned int Sample0 = SampleIndexes[EdgeList[EdgeIndex].x], Sample1 = SampleIndexes[EdgeList[EdgeIndex].y];

			//If one of the samples is infinite, place the vertex using only non-infinite sample's distance
			if (abs(SDFs[Sample0]) == FLT_MAX || abs(SDFs[Sample1]) == FLT_MAX) {
				float dist = abs(SDFs[Sample0]) == FLT_MAX ? SDFs[Sample1] : SDFs[Sample0];
				unsigned int ActiveSample = SDFs[Sample0] == dist ? Sample0 : Sample1, InfiniteSample = SDFs[Sample0] == dist ? Sample1 : Sample0;
				vec3 ActiveSamplePos = SamplePositions[ActiveSample], InfiniteSamplePos = SamplePositions[InfiniteSample];

				VertexBuffer.push_back(ActiveSamplePos + (normalize(ActiveSamplePos - InfiniteSamplePos) * std::min(dist, length(ActiveSamplePos - InfiniteSamplePos))));
			}
			else {
				unsigned int InsideSample = SDFs[Sample0] < SDFs[Sample1] ? Sample0 : Sample1, OutsideSample = SDFs[Sample0] < SDFs[Sample1] ? Sample1 : Sample0;
				vec3 InsideSamplePos = SamplePositions[InsideSample], OutsideSamplePos = SamplePositions[OutsideSample];
				vec3 Inside_to_Outside = OutsideSamplePos - InsideSamplePos;

				vec3 finalpos = InsideSamplePos + (normalize(Inside_to_Outside) * (abs(SDFs[InsideSample]) * length(Inside_to_Outside) / (abs(SDFs[Sample0]) + abs(SDFs[Sample1]))));
				VertexBuffer.push_back(finalpos);
			}
			 
			Table_EdgeSearchIndex++;
		}
		for (unsigned int FaceID = 0; FaceID < (VertexBuffer.size() - VertexBufferSize) / 3 && OutputNormals; FaceID++) {
			vec3 v[3] = { VertexBuffer[VertexBufferSize + (FaceID * 3)] , VertexBuffer[VertexBufferSize + (FaceID * 3) + 1] , VertexBuffer[VertexBufferSize + (FaceID * 3) + 2] };

			if (RefPC) {
				if (!RefPC->PointNormals.size()) { LOG_CRASHING("Invalid input, reference point cloud should have normals!"); return vector<vec3>(); }
				for (unsigned int GeneratedVertexIndex = 0; GeneratedVertexIndex < 3; GeneratedVertexIndex++) {
					vec3 RefNormal = RefPC->PointNormals[0].Normals[TuranEditor::Algorithms::Searchfor_ClosestNeighbors(*RefPC, v[GeneratedVertexIndex], 1)[0]];
					vec3 normal = cross(v[(GeneratedVertexIndex + 1) % 3] - v[GeneratedVertexIndex], v[(GeneratedVertexIndex + 2) % 3] - v[GeneratedVertexIndex]);
					normal *= sign(dot(RefNormal, normal));
					OutputNormals->push_back(normalize(normal));
				}
			}
			else {
				float dist = FLT_MAX;
				unsigned int NearestCornerIndex_toV0 = 0;
				for (unsigned int CornerIndex = 0; CornerIndex < 8; CornerIndex++) {
					if (length(SamplePositions[SampleIndexes[CornerIndex]] - v[0]) < dist) {
						NearestCornerIndex_toV0 = CornerIndex;
						dist = length(SamplePositions[SampleIndexes[CornerIndex]] - v[0]);
					}
				}

				vec3 normal = normalize(cross(v[2] - v[0], v[1] - v[0]));
				normal *= sign(dot(SamplePositions[SampleIndexes[NearestCornerIndex_toV0]] - v[0], normal) * SDFs[SampleIndexes[NearestCornerIndex_toV0]]);
				normal = normalize(normal);
				OutputNormals->push_back(normal);
				OutputNormals->push_back(normal);
				OutputNormals->push_back(normal);
			}


		}

		unsigned int ProgressIndicatorMax = SDFResolution.x * SDFResolution.y * SDFResolution.z > 100000 ? 100000 : 100;
		if (CellIndex > ProgressIndicatorMax && CellIndex % (SDFCellCount / ProgressIndicatorMax) == 0) {
			LOG_STATUS("%" + to_string(float(CellIndex) / (float(SDFCellCount) / 100.0)));
		}
	}
	return VertexBuffer;
}

#include "3rdPartyLibs/project.h"

#define ACCUM_ANGLEOFFSET (int(5))
#define ACCUM_ROFFSET ((int)5)
#define conversion 3.14 / 180.0
void TuranEditor::Algorithms::HoughBasedPlaneDetection(PointCloud* CLOUD, unsigned int DistanceSampleCount, unsigned int AngleSampleCount, unsigned int MaximaThreshold, std::vector<Plane>& FoundPlanes) {
	FoundPlanes.clear();
	if (CLOUD->PointCount == 0) {
		LOG_CRASHING("Point cloud has no points!");
		return;
	}

	
	//Find bounding box of the point cloud
	dvec3 BOUNDINGMIN(DBL_MAX), BOUNDINGMAX(-DBL_MAX);
	double MINDIST(DBL_MAX);
	for (unsigned int i = 0; i < CLOUD->PointCount; i++) {
		vec3 Point = CLOUD->PointPositions[i];
		for (unsigned int DistCheckIndex = i + 1; DistCheckIndex < CLOUD->PointCount; DistCheckIndex++) {
			double dist = length(dvec3(Point) - dvec3(CLOUD->PointPositions[DistCheckIndex]));
			if (dist < MINDIST) { MINDIST = dist; }
		}
		if (Point.x < BOUNDINGMIN.x) { BOUNDINGMIN.x = Point.x; }
		if (Point.y < BOUNDINGMIN.y) { BOUNDINGMIN.y = Point.y; }
		if (Point.z < BOUNDINGMIN.z) { BOUNDINGMIN.z = Point.z; }

		if (Point.x > BOUNDINGMAX.x) { BOUNDINGMAX.x = Point.x; }
		if (Point.y > BOUNDINGMAX.y) { BOUNDINGMAX.y = Point.y; }
		if (Point.z > BOUNDINGMAX.z) { BOUNDINGMAX.z = Point.z; }
	}
	dvec3 CUBESIZE = BOUNDINGMAX - BOUNDINGMIN;
	BOUNDINGMIN -= CUBESIZE / 4.0;
	BOUNDINGMAX += CUBESIZE / 4.0;
	CUBESIZE *= 1.5;
	std::cout << "X: " << CUBESIZE.x << " Y: " << CUBESIZE.y << " Z: " << CUBESIZE.z << "\n";
	if (double(CUBESIZE.x / double(DistanceSampleCount)) < FLT_EPSILON ||
		double(CUBESIZE.y / double(DistanceSampleCount)) < FLT_EPSILON ||
		double(CUBESIZE.z / double(DistanceSampleCount)) < FLT_EPSILON) {
		LOG_CRASHING("Bounding box of the point cloud so small that float precision isn't precise enough! Hough transform has failed!");
		return;
	}
	double MAXEDGELENGTH = std::max(CUBESIZE.x, std::max(CUBESIZE.y, CUBESIZE.z));
	const unsigned int R_ARRAY_SIZE = MAXEDGELENGTH / float(DistanceSampleCount);
	const float SAMPLEDIST = length(CUBESIZE) / double(DistanceSampleCount);



	//XY Angle -> YZ Angle -> Distance
	LOG_STATUS("Creating Hough Space...");
	std::vector<std::vector<std::vector<unsigned int>>> HoughSpace;
	const double AngleDelta_inRadians = glm::radians(180.0 / double(AngleSampleCount));
	HoughSpace.resize(AngleSampleCount);
	for (unsigned int XYAngle_i = 0; XYAngle_i < AngleSampleCount; XYAngle_i++) {
		HoughSpace[XYAngle_i].resize(AngleSampleCount);
		for (unsigned int YZAngle_i = 0; YZAngle_i < AngleSampleCount; YZAngle_i++) {
			HoughSpace[XYAngle_i][YZAngle_i].resize(R_ARRAY_SIZE, 0);
		}
	}

	LOG_STATUS("Filling Hough Space...");
	for (unsigned int XYAngle_i = 0; XYAngle_i < AngleSampleCount; XYAngle_i++) {
		for (unsigned int YZAngle_i = 0; YZAngle_i < AngleSampleCount; YZAngle_i++) {
			const double angle_xy = radians((XYAngle_i + 1) * AngleDelta_inRadians);
			const double angle_yz = radians((YZAngle_i + 1) * AngleDelta_inRadians);
			glm::dvec3 Tangent(glm::cos(angle_xy) * glm::cos(angle_yz), glm::sin(angle_xy) * glm::cos(angle_yz), glm::sin(angle_yz));
			Tangent = glm::normalize(Tangent);
			glm::dvec3 Bitangent = glm::cross(glm::dvec3(0, 1, 0), glm::normalize(Tangent));
			Bitangent = glm::normalize(Bitangent);
			glm::dvec3 Normal = glm::cross(Tangent, Bitangent);
			Normal = glm::normalize(Normal);
			for (unsigned int Dist_ui = 0; Dist_ui < R_ARRAY_SIZE; Dist_ui++) {
				//Index is normalized to move plane to both sides of the center
				const int Dist_i = int(Dist_ui) - int(R_ARRAY_SIZE / 2);
				double AngleBetweenBBDiagonal_andPlaneNormal = dot(normalize(Normal), normalize(CUBESIZE));
				if (AngleBetweenBBDiagonal_andPlaneNormal < 0.0) {
					AngleBetweenBBDiagonal_andPlaneNormal = dot(normalize(-Normal), normalize(CUBESIZE));
				}
				const glm::dvec3 SamplePos = BOUNDINGMIN + glm::dvec3(CUBESIZE / 2.0) + (Normal * glm::dvec3(double(Dist_i) * SAMPLEDIST));
				//Intersect each point with the plane and if the distance is smaller than the discretizer, vote.
				for (unsigned int PointIndex = 0; PointIndex < CLOUD->PointCount; PointIndex++) {
					const glm::dvec3 Center_toPoint = glm::dvec3(CLOUD->PointPositions[PointIndex]) - SamplePos;

					const double dist_fromsurface = length(Center_toPoint) * dot(normalize(Center_toPoint), Normal);
					if (glm::abs(dist_fromsurface) < SAMPLEDIST) {
						HoughSpace[XYAngle_i][YZAngle_i][Dist_ui]++;
					}
				}
			}
		}
	}

	LOG_STATUS("Accumulator is working to find all possible planes (local maximas)!");
	for (int AngleXY_i = 0; AngleXY_i < AngleSampleCount; AngleXY_i++) {
		for (int AngleYZ_i = 0; AngleYZ_i < AngleSampleCount; AngleYZ_i++) {
			double angle_xy = radians((AngleXY_i + 1) * AngleDelta_inRadians);
			double angle_yz = radians((AngleYZ_i + 1) * AngleDelta_inRadians);
			glm::dvec3 Tangent(glm::cos(angle_xy) * glm::cos(angle_yz), glm::sin(angle_xy) * glm::cos(angle_yz), glm::sin(angle_yz));
			Tangent = glm::normalize(Tangent);
			glm::dvec3 Bitangent = glm::cross(glm::dvec3(0, 1, 0), glm::normalize(Tangent));
			Bitangent = glm::normalize(Bitangent);
			glm::dvec3 Normal = glm::cross(Tangent, Bitangent);
			Normal = glm::normalize(Normal);
			//Even though it is int, its values are positive.
			for (int R_ui = 0; R_ui < R_ARRAY_SIZE; R_ui++) {
				unsigned int HOUGHVOTE = HoughSpace[AngleXY_i][AngleYZ_i][R_ui];

				if (HOUGHVOTE < MaximaThreshold) {
					continue;
				}

				//Is that local maxima?
				bool isFoundNewMaxima = false;
				for (int AngleXYOffset = -ACCUM_ANGLEOFFSET; AngleXYOffset <= ACCUM_ANGLEOFFSET && !isFoundNewMaxima; AngleXYOffset++) {
					if (AngleXYOffset + AngleXY_i >= AngleSampleCount || AngleXY_i + AngleXYOffset < 0) {
						continue;
					}
					for (int AngleYZOffset = -ACCUM_ANGLEOFFSET; AngleYZOffset <= ACCUM_ANGLEOFFSET && !isFoundNewMaxima; AngleYZOffset++) {
						if (AngleYZOffset + AngleYZ_i >= AngleSampleCount || AngleYZ_i + AngleYZOffset < 0) {
							continue;
						}
						for (int roffset = -ACCUM_ROFFSET; roffset <= ACCUM_ROFFSET && !isFoundNewMaxima; roffset++) {
							if (R_ui + roffset >= R_ARRAY_SIZE || roffset + roffset < 0) {
								continue;
							}
							unsigned int Check = HoughSpace[AngleXY_i + AngleXYOffset]
								[AngleYZ_i + AngleYZOffset][R_ui + roffset];
							if (Check > HOUGHVOTE) {
								isFoundNewMaxima = true;
								//std::cout << "Found New Maxima at AngleXYOffset: " << AngleXYOffset << " AngleYZOffset: " << AngleYZOffset
									//<< " VoxelX: " << xoffset << " " << yoffset << " " << zoffset << std::endl;
							}
						}
					}
				}

				//If this is not local maxima, skip it
				if (isFoundNewMaxima) {
					continue;
				}

				const int R_i = int(R_ui) - int(R_ARRAY_SIZE / 2);
				double AngleBetweenBBDiagonal_andPlaneNormal = dot(normalize(Normal), normalize(CUBESIZE));
				if (AngleBetweenBBDiagonal_andPlaneNormal < 0.0) {
					AngleBetweenBBDiagonal_andPlaneNormal = dot(normalize(-Normal), normalize(CUBESIZE));
				}
				const dvec3 VoxelLoc = BOUNDINGMIN + glm::dvec3(CUBESIZE / 2.0) + (Normal * glm::dvec3(double(R_i) * SAMPLEDIST));

				Plane possiblePlane;
				possiblePlane.Center = VoxelLoc;
				possiblePlane.Tangent = Tangent;
				possiblePlane.Bitangent = Bitangent;
				possiblePlane.Size = vec2(MAXEDGELENGTH);
				std::cout << "Found Plane Center X: " << possiblePlane.Center.x << " Y: " << possiblePlane.Center.y << " Z: " << possiblePlane.Center.z <<
					"\Tangent X: " << possiblePlane.Tangent.x << " Y: " << possiblePlane.Tangent.y << " Z: " << possiblePlane.Tangent.z << " Vote: " << HOUGHVOTE << "\n\n";
				bool isFound = false;
				for (unsigned int previousPossiblePlane_i = 0; previousPossiblePlane_i < FoundPlanes.size() && !isFound; previousPossiblePlane_i++) {
					if (FoundPlanes[previousPossiblePlane_i].Center.x == float(VoxelLoc.x) && FoundPlanes[previousPossiblePlane_i].Center.y == float(VoxelLoc.y) &&
						FoundPlanes[previousPossiblePlane_i].Tangent.x == float(Tangent.x) && FoundPlanes[previousPossiblePlane_i].Tangent.y == float(Tangent.y) &&
						FoundPlanes[previousPossiblePlane_i].Tangent.z == float(Tangent.z)) {
						isFound = true;
					}
				}
				if (!isFound) {
					std::cout << "Found a new plane with vote: " << HOUGHVOTE << std::endl;
					FoundPlanes.push_back(possiblePlane);
				}
			}
		}
	}
}


struct HoughBasedPlaneDetection_VARs {
	unsigned int PointCount = 0;
	dvec3 BOUNDINGMIN = dvec3(DBL_MAX), BOUNDINGMAX = dvec3(DBL_MIN), CUBESIZE = dvec3(DBL_MIN);
	double MINDIST_BETWEENPOINTS = double(DBL_MAX), DISCRETIZER = DBL_MAX;
	unsigned int R_ARRAY_SIZE = UINT32_MAX;
	std::vector<std::vector<std::vector<std::vector<unsigned int>>>> HoughSpace;

	TuranEditor::POINTRENDERER* SamplePoints = nullptr;
	//FILLING HOUGH SPACE
	unsigned int FHS_XYAnglei = 0, FHS_YZAnglei = 0, FHS_VoxelXi = 0, FHS_VoxelYi = 0;
};

void ClearProgressiveInfo_HoughBasedPlaneDetection(HoughBasedPlaneDetection_VARs& var) {

}

bool TuranEditor::Algorithms::Progressive_HoughBasedPlaneDetection(PointCloud* CLOUD, unsigned int BoundingCubeRes, unsigned int AngleSampleCount,
	unsigned int MaximaThreshold, unsigned int& Step, bool ShouldContinue, vector<GFX_API::SpecialDrawCall>& SpecialDrawCallBuffer, TuranEditor::POINTRENDERER* PCRenderer) {
	if (CLOUD->PointCount == 0) {
		LOG_CRASHING("Point cloud has no points!");
		return true;
	}
	

	static HoughBasedPlaneDetection_VARs vars;

	if (vars.PointCount != CLOUD->PointCount) {
		ClearProgressiveInfo_HoughBasedPlaneDetection(vars);
	}

	const double AngleDelta_inRadians = glm::radians(180.0 / double(AngleSampleCount));
	//Find bounding box of the point cloud if it's not calculated before
	if (vars.MINDIST_BETWEENPOINTS == DBL_MAX) {
		for (unsigned int i = 0; i < CLOUD->PointCount; i++) {
			vec3 Point = CLOUD->PointPositions[i];
			for (unsigned int DistCheckIndex = i + 1; i < CLOUD->PointCount; i++) {
				double dist = length(dvec3(Point) - dvec3(CLOUD->PointPositions[DistCheckIndex]));
				if (dist < vars.MINDIST_BETWEENPOINTS) { vars.MINDIST_BETWEENPOINTS = dist; }
			}
			if (Point.x < vars.BOUNDINGMIN.x) { vars.BOUNDINGMIN.x = Point.x; }
			if (Point.y < vars.BOUNDINGMIN.y) { vars.BOUNDINGMIN.y = Point.y; }
			if (Point.z < vars.BOUNDINGMIN.z) { vars.BOUNDINGMIN.z = Point.z; }

			if (Point.x > vars.BOUNDINGMAX.x) { vars.BOUNDINGMAX.x = Point.x; }
			if (Point.y > vars.BOUNDINGMAX.y) { vars.BOUNDINGMAX.y = Point.y; }
			if (Point.z > vars.BOUNDINGMAX.z) { vars.BOUNDINGMAX.z = Point.z; }
		}
		vars.BOUNDINGMIN.x = vars.BOUNDINGMIN.x > 0.0 ? vars.BOUNDINGMIN.x / 1.25 : vars.BOUNDINGMIN.x * 1.25;
		vars.BOUNDINGMIN.y = vars.BOUNDINGMIN.y > 0.0 ? vars.BOUNDINGMIN.y / 1.25 : vars.BOUNDINGMIN.y * 1.25;
		vars.BOUNDINGMIN.z = vars.BOUNDINGMIN.z > 0.0 ? vars.BOUNDINGMIN.z / 1.25 : vars.BOUNDINGMIN.z * 1.25;
		vars.BOUNDINGMAX.x = vars.BOUNDINGMAX.x < 0.0 ? vars.BOUNDINGMAX.x / 1.25 : vars.BOUNDINGMAX.x * 1.25;
		vars.BOUNDINGMAX.x = vars.BOUNDINGMAX.x < 0.0 ? vars.BOUNDINGMAX.x / 1.25 : vars.BOUNDINGMAX.x * 1.25;
		vars.BOUNDINGMAX.x = vars.BOUNDINGMAX.x < 0.0 ? vars.BOUNDINGMAX.x / 1.25 : vars.BOUNDINGMAX.x * 1.25;
		vars.CUBESIZE = vars.BOUNDINGMAX - vars.BOUNDINGMIN;

		if (double(vars.CUBESIZE.x / double(BoundingCubeRes)) < FLT_EPSILON ||
			double(vars.CUBESIZE.y / double(BoundingCubeRes)) < FLT_EPSILON ||
			double(vars.CUBESIZE.z / double(BoundingCubeRes)) < FLT_EPSILON) {
			LOG_CRASHING("Bounding box of the point cloud so small that float precision isn't precise enough! Hough transform has failed!");
			return true;
		}
		dvec3 VOXELDIST = vars.CUBESIZE / dvec3(BoundingCubeRes);
		vars.DISCRETIZER = std::min(VOXELDIST.x, std::min(VOXELDIST.y, VOXELDIST.z));

		double MAXEDGELENGTH = std::max(vars.CUBESIZE.x, std::max(vars.CUBESIZE.y, vars.CUBESIZE.z));
		vars.R_ARRAY_SIZE = MAXEDGELENGTH / vars.DISCRETIZER;


		//XY Angle -> YZ Angle -> VoxelX -> VoxelY -> VoxelZ
		LOG_STATUS("Creating Hough Space...");
		
		vars.HoughSpace.resize(AngleSampleCount);
		for (unsigned int XYAngle_i = 0; XYAngle_i < AngleSampleCount; XYAngle_i++) {
			vars.HoughSpace[XYAngle_i].resize(AngleSampleCount);
			for (unsigned int YZAngle_i = 0; YZAngle_i < AngleSampleCount; YZAngle_i++) {
				vars.HoughSpace[XYAngle_i][YZAngle_i].resize(BoundingCubeRes);
				for (unsigned int VoxelX_i = 0; VoxelX_i < BoundingCubeRes; VoxelX_i++) {
					vars.HoughSpace[XYAngle_i][YZAngle_i][VoxelX_i].resize(BoundingCubeRes, 0);
				}
			}
		}



		//Render bounding box and sample points here
		//Then leave the function, because this is the breakpoint
		SpecialDrawCallBuffer.push_back(TuranEditor::RenderDataManager::Create_BoundingBoxSpecialDrawCall(vars.BOUNDINGMIN, vars.BOUNDINGMAX));
		const unsigned int SamplePointsCount = BoundingCubeRes * BoundingCubeRes;
		vars.SamplePoints = TuranEditor::RenderDataManager::Create_PointRenderer(SamplePointsCount);
		for (unsigned int VoxelX_i = 0; VoxelX_i < BoundingCubeRes; VoxelX_i++) {
			for (unsigned int VoxelY_i = 0; VoxelY_i < BoundingCubeRes; VoxelY_i++) {
				const unsigned int Access_i = (VoxelX_i * BoundingCubeRes) + (VoxelY_i * BoundingCubeRes);
				vars.SamplePoints->GetPointPosition_byIndex(Access_i) = vars.BOUNDINGMIN + glm::dvec3(VoxelX_i * VOXELDIST.x, VoxelY_i * VOXELDIST.y, 0);
				vars.SamplePoints->GetPointCOLORRGBA_byIndex(Access_i) = vec4(0.0f);
			}
		}
		return false;
	}

	if (ShouldContinue == false) {
		return false;
	}


	LOG_STATUS("Filling Hough Space...");
	for (vars.FHS_XYAnglei; vars.FHS_XYAnglei < AngleSampleCount; vars.FHS_XYAnglei++) {
		for (vars.FHS_YZAnglei; vars.FHS_YZAnglei < AngleSampleCount; vars.FHS_YZAnglei++) {
			const double angle_xy = (vars.FHS_XYAnglei * AngleDelta_inRadians);
			const double angle_yz = (vars.FHS_YZAnglei * AngleDelta_inRadians);
			glm::dvec3 Tangent(glm::cos(angle_xy) * glm::cos(angle_yz), glm::sin(angle_xy) * glm::cos(angle_yz), glm::sin(angle_yz));
			Tangent = glm::normalize(Tangent);
			glm::dvec3 Bitangent = glm::cross(glm::dvec3(0, 1, 0), glm::normalize(Tangent));
			Bitangent = glm::normalize(Bitangent);
			glm::dvec3 Normal = glm::cross(Tangent, Bitangent);
			Normal = glm::normalize(Normal);
			for (vars.FHS_VoxelXi; vars.FHS_VoxelXi < BoundingCubeRes; vars.FHS_VoxelXi++) {
				for (vars.FHS_VoxelYi; vars.FHS_VoxelYi < BoundingCubeRes; vars.FHS_VoxelYi++) {
					/*
					const glm::dvec3 SamplePos = vars.BOUNDINGMIN + glm::dvec3(vars.FHS_VoxelXi * VOXELDIST.x, vars.FHS_VoxelYi * VOXELDIST.y, 0);

					//Intersect each point with the plane and if the distance is smaller than the discretizer, vote.
					for (unsigned int PointIndex = 0; PointIndex < CLOUD->PointCount; PointIndex++) {
						const glm::dvec3 Center_toPoint = glm::dvec3(CLOUD->PointPositions[PointIndex]) - SamplePos;

						const double dist_fromsurface = length(Center_toPoint) * dot(normalize(Center_toPoint), Normal);
						if (glm::abs(dist_fromsurface) < DISCRETIZER) {
							HoughSpace[XYAngle_i][YZAngle_i][VoxelX_i][VoxelY_i]++;
						}
					}*/
				}


			}
		}
	}

}


#include <libqhullcpp/Qhull.h>
#include <libqhullcpp/QhullLinkedList.h>
#include <libqhullcpp/QhullPoints.h>
#include <libqhullcpp/QhullFacetList.h>
#include <libqhullcpp/QhullFacetSet.h>
#include <libqhullcpp/QhullUser.h>
#include <libqhullcpp/QhullIterator.h>
#include <libqhullcpp/QhullSet.h>
#include <libqhullcpp/QhullVertexSet.h>
/*
unsigned int TuranEditor::Algorithms::VoronoiDiagram::AddVoronoiEdge(unsigned int EdgePoint0, unsigned int EdgePoint1) {
	if (EdgePoint0 >= VoronoiVertices.size() || EdgePoint1 >= VoronoiVertices.size()) {
		LOG_CRASHING("AddVoronoiEdge() is called with overflowing input!");
		return UINT32_MAX;
	}
	for (unsigned int i = 0; i < VoronoiEdges.size(); i++) {
		if((VoronoiEdges[i].EdgePoints[0] == EdgePoint0 && VoronoiEdges[i].EdgePoints[1] == EdgePoint1) ||
			(VoronoiEdges[i].EdgePoints[1] == EdgePoint0 && VoronoiEdges[i].EdgePoints[0] == EdgePoint1)){
			return i;
		}
	}
	VoronoiEdge edge;
	edge.EdgePoints[0] = EdgePoint0;
	edge.EdgePoints[1] = EdgePoint1;
	VoronoiEdges.push_back(edge);
	return VoronoiEdges.size() - 1;
}*/
TuranEditor::Algorithms::VoronoiDiagram* TuranEditor::Algorithms::CreateVoronoiDiagram(PointCloud* CLOUD) {
	orgQhull::Qhull x;
	static constexpr unsigned int Dim = 3;
	


	vector<double> points(CLOUD->PointCount * Dim, DBL_MAX);
	for (unsigned int i = 0; i < CLOUD->PointCount; i++) {
		points[(i * 3)] = CLOUD->PointPositions[i].x;
		points[(i * 3) + 1] = CLOUD->PointPositions[i].y;
		points[(i * 3) + 2] = CLOUD->PointPositions[i].z;
	}
	x.runQhull("", Dim, CLOUD->PointCount, points.data(), "v o Fv");                                                         

	bool isLower;
	int VertexCount;
	x.prepareVoronoi(&isLower, &VertexCount);
	int voronoiDimension = x.hullDimension() - 1;
	int numfacets = x.facetCount();
	size_t numpoints = x.points().size();



	// Gather Voronoi vertices
	std::vector<std::vector<double> > voronoiVertices;
	std::vector<double> vertexAtInfinity;
	for (int i = 0; i < Dim; ++i) {
		vertexAtInfinity.push_back(qh_INFINITE);
	}
	voronoiVertices.push_back(vertexAtInfinity);
	// for(QhullFacet facet : qhull.facetList())
	orgQhull::QhullLinkedListIterator<orgQhull::QhullFacet> j(x.facetList());
	while (j.hasNext()) {
		orgQhull::QhullFacet facet = j.next();
		if (facet.visitId() && facet.visitId() < numfacets) {
			voronoiVertices.push_back(facet.getCenter().toStdVector());
		}
	}
	LOG_STATUS("Number of voronoi vertices: " + std::to_string(voronoiVertices.size()));


	TuranEditor::Algorithms::VoronoiDiagram* diagram = new TuranEditor::Algorithms::VoronoiDiagram;
	//0th voronoi vertex is infinity, so skip that
	diagram->VoronoiVertices.resize(voronoiVertices.size() - 1);
	for (unsigned int i = 0; i < diagram->VoronoiVertices.size(); i++) {
		diagram->VoronoiVertices[i] = vec3(voronoiVertices[i + 1][0], voronoiVertices[i + 1][1], voronoiVertices[i + 1][2]);
	}


	
	// Gather Voronoi regions
	std::vector<std::vector<int> > voronoiRegions(numpoints);
	orgQhull::QhullVertexListIterator j2(x.vertexList());
	while (j2.hasNext()) {
		orgQhull::QhullVertex vertex = j2.next();
		size_t numinf = 0;
		std::vector<int> voronoiRegion;
		orgQhull::QhullFacetSetIterator k2(vertex.neighborFacets());
		while (k2.hasNext()) {
			orgQhull::QhullFacet neighbor = k2.next();
			if (neighbor.visitId() == 0) {
				if (!numinf) {
					numinf = 1;
					voronoiRegion.push_back(0); // the voronoiVertex at infinity indicates an unbounded region
				}
			}
			else if (neighbor.visitId() < numfacets) {
				voronoiRegion.push_back(neighbor.visitId());
			}
		}
		if (voronoiRegion.size() > numinf) {
			int siteId = vertex.point().id();
			if (siteId >= 0 && siteId<int(numpoints)) { // otherwise indicate qh.other_points
				voronoiRegions[siteId] = voronoiRegion;
			}
		}
	}
		
	
	diagram->VoronoiRegions.resize(voronoiRegions.size());
	for (unsigned int RegionIndex = 0; RegionIndex < diagram->VoronoiRegions.size(); RegionIndex++) {
		//Count the bounded linked vertexes (so skip the infinity vertices)
		for (unsigned int j = 0; j < voronoiRegions[RegionIndex].size(); j++) {
			if (voronoiRegions[RegionIndex][j] == 0) {
				continue;
			}
			diagram->VoronoiRegions[RegionIndex].VertexCount++;
		}
		diagram->VoronoiRegions[RegionIndex].VertexIDs = new unsigned int[diagram->VoronoiRegions[RegionIndex].VertexCount];
		unsigned int k = 0;
		for (unsigned int j = 0; j < voronoiRegions[RegionIndex].size(); j++) {
			if (voronoiRegions[RegionIndex][j] == 0) {
				continue;
			}
			diagram->VoronoiRegions[RegionIndex].VertexIDs[k] = voronoiRegions[RegionIndex][j] - 1;
			k++;
		}
		vector<double> VertexPositions(diagram->VoronoiRegions[RegionIndex].VertexCount * 3, 0.0);
		for (unsigned int RegionsVertexIndex = 0; RegionsVertexIndex < diagram->VoronoiRegions[RegionIndex].VertexCount; RegionsVertexIndex++) {
			VertexPositions[RegionsVertexIndex * 3] = diagram->VoronoiVertices[diagram->VoronoiRegions[RegionIndex].VertexIDs[RegionsVertexIndex]].x;
			VertexPositions[(RegionsVertexIndex * 3) + 1] = diagram->VoronoiVertices[diagram->VoronoiRegions[RegionIndex].VertexIDs[RegionsVertexIndex]].y;
			VertexPositions[(RegionsVertexIndex * 3) + 2] = diagram->VoronoiVertices[diagram->VoronoiRegions[RegionIndex].VertexIDs[RegionsVertexIndex]].z;
		}
		orgQhull::Qhull convexer("", 3, diagram->VoronoiRegions[RegionIndex].VertexCount, VertexPositions.data(), "Qt o");

		orgQhull::QhullFacetList facets = convexer.facetList();
		std::vector<std::vector<int> > facetVertices;
		// for(QhullFacet f : facets)
		orgQhull::QhullFacetListIterator j(facets);
		while (j.hasNext()) {
			orgQhull::QhullFacet f = j.next();
			std::vector<int> vertices;
			if (!f.isGood()) {
				// ignore facet
			}
			else if (!f.isTopOrient() && f.isSimplicial()) { /* orient the vertices like option 'o' */
				orgQhull::QhullVertexSet vs = f.vertices();
				vertices.push_back(vs[1].point().id());
				vertices.push_back(vs[0].point().id());
				for (int i = 2; i < (int)vs.size(); ++i) {
					vertices.push_back(vs[i].point().id());
				}
				facetVertices.push_back(vertices);
			}
			else {  /* note: for non-simplicial facets, this code does not duplicate option 'o', see qh_facet3vertex and qh_printfacetNvertex_nonsimplicial */
			   // for(QhullVertex vertex : f.vertices()){
				orgQhull::QhullVertexSetIterator k(f.vertices());
				while (k.hasNext()) {
					orgQhull::QhullVertex vertex = k.next();
					orgQhull::QhullPoint p = vertex.point();
					vertices.push_back(p.id());
				}
				facetVertices.push_back(vertices);
			}
		}
		// for(std::vector<int> vertices : facetVertices)
		for (size_t k = 0; k < facetVertices.size(); ++k) {
			const std::vector<int>& vertices = facetVertices[k];
			size_t n = vertices.size();
			//cout << n << " ";
			for (size_t i = 0; i < n; ++i) {
				//cout << vertices[i] << " ";
			}
			//cout << "\n";
			if (vertices.size() != 3) {
				int ZaWarudo = 0;
				std::cout << "One of the voronoi region convex hulls failed to form a triangle mesh!\n";
				std::cin >> ZaWarudo;
			}

			/*
			diagram->VoronoiRegions[RegionIndex].EdgeIDs.push_back(diagram->AddVoronoiEdge(VertexIDs[0], VertexIDs[2]));
			diagram->VoronoiRegions[RegionIndex].EdgeIDs.push_back(diagram->AddVoronoiEdge(VertexIDs[0], VertexIDs[2]));
			diagram->VoronoiRegions[RegionIndex].EdgeIDs.push_back(diagram->AddVoronoiEdge(VertexIDs[0], VertexIDs[1]));*/
			VoronoiDiagram::VoronoiTriangle tri;
			tri.Vertexes[0] = vec3(VertexPositions[vertices[0] * 3], VertexPositions[(vertices[0] * 3) + 1], VertexPositions[(vertices[0] * 3) + 2]);
			tri.Vertexes[1] = vec3(VertexPositions[vertices[1] * 3], VertexPositions[(vertices[1] * 3) + 1], VertexPositions[(vertices[1] * 3) + 2]);
			tri.Vertexes[2] = vec3(VertexPositions[vertices[2] * 3], VertexPositions[(vertices[2] * 3) + 1], VertexPositions[(vertices[2] * 3) + 2]);
			diagram->VoronoiTriangles.push_back(tri);
			diagram->VoronoiRegions[RegionIndex].TriangleIDs.push_back(diagram->VoronoiTriangles.size() - 1);
		}
	}
	
	return diagram;
}