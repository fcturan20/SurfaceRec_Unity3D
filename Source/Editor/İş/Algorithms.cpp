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
vec3 TuranEditor::Algorithms::Searchfor_ClosestPoint(PointCloud& cloud, vec3& Point, long long* timing) {
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

vector<unsigned int> TuranEditor::Algorithms::Searchfor_ClosestNeighbors(const PointCloud& cloud, vec3& Point, unsigned int numberofneighbors, long long* timing) {
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
	TAPIFILESYSTEM::Overwrite_BinaryFile(PATH, data_ptr, data_size);
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

	return MESHBUFFER_ID;
}
