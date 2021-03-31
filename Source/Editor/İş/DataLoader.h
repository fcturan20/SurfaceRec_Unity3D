#pragma once
#include "DataTypes.h"

namespace TuranEditor {

	class DataLoader {
	public:
		static PointCloud* LoadMesh_asPointCloud(const char* PATH);
	};
}