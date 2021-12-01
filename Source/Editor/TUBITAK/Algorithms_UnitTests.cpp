#include "Algorithms_UnitTests.h"
#include "TuranAPI/Logger_Core.h"
#include "TuranAPI/Profiler_Core.h"

namespace TuranEditor {

	bool Does_PointLists_match(vector<vec3>& List1, vector<vec3>& List2) {
		if (List1.size() != List2.size()) {
			LOG_ERROR("Sizes don't match!");
			return false;
		}
		vector<bool> UnfoundItems1(List1.size());
		vector<bool> UnfoundItems2(List2.size());
		bool exactmatch = true;
		for (unsigned int List1Index = 0; List1Index < List1.size(); List1Index++) {
			bool isFound = false;
			for (unsigned int List2Index = 0; List2Index < List2.size(); List2Index++) {
				if (List1[List1Index] == List2[List2Index]) {
					UnfoundItems2[List2Index] = true;
					isFound = true;
					break;
				}
			}
			UnfoundItems1[List1Index] = isFound;
			if (!isFound) {
				exactmatch = false;
			}
		}
		for (unsigned int UnfoundItems1Index = 0; UnfoundItems1Index < UnfoundItems1.size(); UnfoundItems1Index++) {
			if (UnfoundItems1[UnfoundItems1Index]) {
				continue;
			}
			vec3 item1 = List1[UnfoundItems1Index];
			float mindist = FLT_MAX;
			unsigned int closestelement = UINT32_MAX;
			for (unsigned int UnfoundItems2SearchIndex = 0; UnfoundItems2SearchIndex < UnfoundItems2.size(); UnfoundItems2SearchIndex++) {
				if (!UnfoundItems2[UnfoundItems2SearchIndex] && mindist > length(item1 - List2[UnfoundItems2SearchIndex])) {
					mindist = length(item1 - List2[UnfoundItems2SearchIndex]);
					closestelement = UnfoundItems2SearchIndex;
				}
			}
			LOG_STATUS("Length between unfound items: " + to_string(mindist) + " and their indices are: " + to_string(UnfoundItems1Index) + " & " + to_string(closestelement));
		}
		return exactmatch;
	}
	bool AlgorithmUnitTests::Test_KDTree_PointCloud(PointCloud& Cloud) {
		vec3 MinValues(FLT_MAX, FLT_MAX, FLT_MAX), MaxValues(FLT_MIN, FLT_MIN, FLT_MIN);
		for (unsigned int PointIndex = 0; PointIndex < Cloud.PointCount; PointIndex++) {
			vec3& Point = Cloud.PointPositions[PointIndex];
			if (Point.x < MinValues.x) {
				MinValues.x = Point.x;
			}
			if (Point.y < MinValues.y) {
				MinValues.y = Point.y;
			}
			if (Point.z < MinValues.z) {
				MinValues.z = Point.z;
			}

			if (Point.x > MaxValues.x) {
				MaxValues.x = Point.x;
			}
			if (Point.y > MaxValues.y) {
				MaxValues.y = Point.y;
			}
			if (Point.z > MaxValues.z) {
				MaxValues.z = Point.z;
			}
		}

		MinValues -= vec3(1.0f, 1.0f, 1.0f);
		MaxValues += vec3(1.0f, 1.0f, 1.0f);

		Algorithms::Generate_KDTree(Cloud);


		unsigned int KNearestFailCount = 0, KDTreeFaster_Count = 0, BruteForceFaster_Count = 0;
		float FailPercentage = 0;
		LOG_CRASHING("Point Count: " + to_string(Cloud.PointCount));
		for (unsigned int PointIndex = 0; PointIndex < Cloud.PointCount; PointIndex++) {
			vec3 SearchPoint = Cloud.PointPositions[PointIndex];
			long long BruteForce_Duration = 0, kdTree_Duration = 0;
			vector<vec3> BruteForce_Closest = Algorithms::Searchfor_ClosestNeighbors(Cloud, SearchPoint, 3, true);
			vector<vec3> kdtree_Closest = Algorithms::Searchfor_ClosestNeighbors(Cloud, SearchPoint, 3, true);
			if (!Does_PointLists_match(kdtree_Closest, BruteForce_Closest)) {
				LOG_STATUS("Closest point failed!");
				KNearestFailCount++;
			}
		}

		LOG_CRASHING("Fail count: " + to_string(KNearestFailCount));
		return true;
	}

	bool AlgorithmUnitTests::Test_DeluanayTriangulation() {
		vector<vec3> Points = { vec3(-5, -5, 0), vec3(-5, 5, 0), vec3(5, -5, 0), vec3(5,5,0) };
		vector<vec3> PCA = Algorithms::Compute_PCA(Points);
		vector<vec3> FinalPoints = Algorithms::ProjectPoints_OnPlane_thenTriangulate(Points, PCA[1], PCA[0], PCA[2]);
		for (unsigned int i = 0; i < FinalPoints.size() / 3; i++) {
			for (unsigned char j = 0; j < 3; j++) {
				LOG_STATUS("Vertex " + to_string(j) + " position: " + to_string(FinalPoints[i * 3 + j].x) + " & " + to_string(FinalPoints[i * 3 + j].y) + " & " + to_string(FinalPoints[i * 3 + j].z));
			}
		}
		return true;
	}
}