#pragma once
#include "Algorithms.h"

namespace TuranEditor {
	class AlgorithmUnitTests {
	public:
		static bool Test_KDTree_PointCloud(PointCloud& Cloud);
		static bool Test_DeluanayTriangulation();
	};

}