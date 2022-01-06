#include "PointCloudTools.h"
#include "Editor/TUBITAK/Algorithms.h"

static string NORMALNAME;
static PCViewer* viewer = nullptr;


void PointCloudTools::NormalEstimation(PCViewer* Viewer) {
	viewer = Viewer;
	if (!IMGUI->Begin_TabBar()) {
		return;
	}

	if (IMGUI->Begin_TabItem("PCA")) {
		static unsigned int selectedlistitemindex = 0, selectedddindex = 0;
		Viewer->SelectListOneLine_FromDisplayableDatas(PCViewer::DisplayableData::DataType::POINTCLOUD, selectedlistitemindex, selectedddindex, "Point Cloud to Calculate Voronoi");
		if (selectedddindex != UINT32_MAX) {
			static int kNNCount = 2;
			IMGUI->Slider_Int("kNN Count for PCA", &kNNCount, 2, 100);
			if (IMGUI->Button("Compute PCA")) {
				PCViewer::PointCloud_DD* pc_dd = static_cast<PCViewer::PointCloud_DD*>(Viewer->DisplayableDatas[selectedddindex]);
				TuranEditor::Algorithms::Generate_KDTree(pc_dd->PC);
				PC_PointNormals pcaresult;
				pcaresult.NAME = "PCA " + to_string(kNNCount);
				pcaresult.Normals = new vec3[pc_dd->PC.PointCount];
				for (unsigned int PointIndex = 0; PointIndex < pc_dd->PC.PointCount; PointIndex++) {
					vector<unsigned int> kNN_indexes = TuranEditor::Algorithms::Searchfor_ClosestNeighbors(pc_dd->PC, pc_dd->PC.PointPositions[PointIndex], kNNCount);
					vector<vec3> kNNPositions(kNN_indexes.size());
					for (unsigned int knnIndex = 0; knnIndex < kNN_indexes.size(); knnIndex++) {
						kNNPositions[knnIndex] = pc_dd->PC.PointPositions[kNN_indexes[knnIndex]];
					}

					pcaresult.Normals[PointIndex] = TuranEditor::Algorithms::Compute_PCA(kNNPositions)[2];
				}

				pc_dd->PC.PointNormals.push_back(pcaresult);
			}
		}
		else {
			IMGUI->Text("Please provide a point cloud!");
		}
		IMGUI->End_TabItem();
	}



	IMGUI->End_TabBar();
}