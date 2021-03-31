#include "Algorithms.h"
#include "TuranAPI/Logger_Core.h"
#include "TuranAPI/Profiler_Core.h"
#include <array>
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include "delaunator.hpp"
using namespace TuranAPI;

#include "kdtree.h"
//#include "GoogleKDTREE.h"
//#include "KDTree.hpp"

namespace TuranEditor {
	class MyPoint : public std::array<float, 3>
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

	void Algorithms::Generate_KDTree(PointCloud& Cloud) {
		if (!Cloud.PointCount) {
			LOG_ERROR("Cloud is empty!");
			return;
		}


		/*	Google KDTree
		kdtree* kd = kd_create(3);
		for (unsigned int PointIndex = 0; PointIndex < Cloud.PointCount; PointIndex++) {
			kd_insertf(kd, (float*)&Cloud.PointPositions[PointIndex], nullptr);
		}*/

		/*
		pointVec cloud_kdtreevector;
		cloud_kdtreevector.resize(Cloud.PointCount);
		for (unsigned int PointIndex = 0; PointIndex < Cloud.PointCount; PointIndex++) {
			cloud_kdtreevector[PointIndex] = { Cloud.PointPositions[PointIndex].x, Cloud.PointPositions[PointIndex].y, Cloud.PointPositions[PointIndex].z };
		}
		kd = new KDTree(cloud_kdtreevector);
		*/
		
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
	vec3 Algorithms::Searchfor_ClosestPoint(PointCloud& cloud, vec3& Point, long long* timing) {
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

	vector<vec3> Algorithms::Searchfor_ClosestNeighbors(PointCloud& cloud, vec3& Point, unsigned int numberofneighbors, bool KDTreeSearch, long long* timing) {
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

	vector<vec3> Algorithms::CrustAlgorithm(vector<vec3>& pointlist) {
		vector<vec3> PCA_dimensions = Compute_PCA(pointlist);
	}

	vector<vec3> Algorithms::Compute_PCA(const vector<vec3>& points) {
		vector<vec3> PCA_Vectors(3);
		vec3 CenterOfMass(0.0f,0.0f,0.0f);
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
	vector<vec3> Algorithms::ProjectPoints_OnPlane_thenTriangulate(const vector<vec3>& points, vec3 Tangent, vec3 Bitangent, vec3 Normal) {
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

		vector<vec2> Points_onPlane(points.size());
		for (unsigned char PointIndex = 0; PointIndex < points.size(); PointIndex++) {
			vec3 Center_toPoint = points[PointIndex] - CenterOfMass;

			float dist_fromsurface = length(Center_toPoint) * dot(normalize(Center_toPoint), Normal);
			vec3 projected_3d = Center_toPoint - Normal * dist_fromsurface;

			vec2& final_2d = Points_onPlane[PointIndex];
			final_2d.x = length(projected_3d) * dot(normalize(projected_3d), Tangent);
			final_2d.y = length(projected_3d) * dot(normalize(projected_3d), Bitangent);
		}

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

				for (unsigned int v_index = 0; v_index < 3; v_index++) {
					vec2 v0_in2d = vec2(d.coords[2 * d.triangles[triangle_index + v_index]], d.coords[2 * d.triangles[triangle_index + v_index] + 1]);
					vertexes[v_index] = (v0_in2d.x * Tangent) + v0_in2d.y * Bitangent + CenterOfMass;
				}

				Triangulation[triangle_index] = vertexes[0];
				Triangulation[triangle_index + 1] = vertexes[1];
				Triangulation[triangle_index + 2] = vertexes[2];
			}
		}
		return Triangulation;
	}
}