#include "PointCloudTools.h"
#include "GFX/GFX_Core.h"
#include "Editor/TUBITAK/Algorithms.h"

//Sphere Detection Variables

static double HOUGHSPHERE_MinSphereRadius = 0.0;
static int HOUGHSPHERE_BoundingCubeRes = 10, HOUGHSPHERE_MinLocalMaximaThreshold = 15;
static std::vector<glm::vec4> HOUGH_Spheres;




void PointCloudTools::ObjectDetection(PCViewer* Viewer) {
	if (!IMGUI->Begin_TabBar()) { return; }
	//Hough Transform based Sphere Detection
	static unsigned int selectedlistitemindex = 0, selectedddindex = 0;
	Viewer->SelectListOneLine_FromDisplayableDatas(PCViewer::DisplayableData::DataType::POINTCLOUD, selectedlistitemindex, selectedddindex, "Point Cloud to Detect Object");
	if (selectedddindex == UINT32_MAX) { IMGUI->End_TabBar(); return; }
	if (IMGUI->Begin_TabItem("Sphere Detection")) {
		PCViewer::PointCloud_DD* PC = static_cast<PCViewer::PointCloud_DD*>(Viewer->DisplayableDatas[selectedddindex]);
		IMGUI->Slider_Int("Bounding Cube Resolution", &HOUGHSPHERE_BoundingCubeRes, 6, 50);
		IMGUI->Slider_Int("Minimum Local Maxima Threshold", &HOUGHSPHERE_MinLocalMaximaThreshold, 50, 5000);

		float Slider_MinSphereRad = HOUGHSPHERE_MinSphereRadius;
		IMGUI->Slider_Float("Minimum Sphere Radius", &Slider_MinSphereRad, 0.0, 1.0);
		HOUGHSPHERE_MinSphereRadius = Slider_MinSphereRad;

		if (IMGUI->Button("Calculate")) {
			TuranEditor::Algorithms::HoughBasedSphereDetection(&PC->PC, HOUGHSPHERE_MinSphereRadius, HOUGHSPHERE_BoundingCubeRes, HOUGHSPHERE_MinLocalMaximaThreshold, HOUGH_Spheres);

			PCViewer::TriangleModel* trimodel = new PCViewer::TriangleModel;
			trimodel->DisplayedVertexBuffers.resize(HOUGH_Spheres.size(), true);
			trimodel->GPUMESH_IDs.resize(HOUGH_Spheres.size(), UINT32_MAX);

			vector<vector<vec3>> FoundSphere_VertexPositions(HOUGH_Spheres.size()), FoundSphere_VertexNormals(HOUGH_Spheres.size()), FoundSphere_VertexBuffers(HOUGH_Spheres.size());
			for (unsigned int Sphere_i = 0; Sphere_i < HOUGH_Spheres.size(); Sphere_i++) {
				vector<vec3> VertexPositions = TuranEditor::RenderDataManager::Create_Sphere(
					glm::vec3(HOUGH_Spheres[Sphere_i].x, HOUGH_Spheres[Sphere_i].y, HOUGH_Spheres[Sphere_i].z),
					HOUGH_Spheres[Sphere_i].w);
				FoundSphere_VertexPositions[Sphere_i] = VertexPositions;
				vector<vec3> VertexNormals(VertexPositions.size(), vec3(0.0));
				for (unsigned int i = 0; i < VertexPositions.size(); i++) {
					VertexNormals[i] = normalize(VertexPositions[i] - glm::vec3(HOUGH_Spheres[Sphere_i].x, HOUGH_Spheres[Sphere_i].y, HOUGH_Spheres[Sphere_i].z));
				}
				FoundSphere_VertexNormals[Sphere_i] = VertexNormals;
				FoundSphere_VertexBuffers[Sphere_i].insert(FoundSphere_VertexBuffers[Sphere_i].begin(), VertexPositions.begin(), VertexPositions.end());
				FoundSphere_VertexBuffers[Sphere_i].insert(FoundSphere_VertexBuffers[Sphere_i].begin() + VertexPositions.size(), VertexNormals.begin(), VertexNormals.end());
				trimodel->GPUMESH_IDs[Sphere_i] = GFXContentManager->Upload_MeshBuffer(TuranEditor::RenderDataManager::PositionNormal_VertexAttrib, FoundSphere_VertexBuffers[Sphere_i].data(),
					FoundSphere_VertexBuffers[Sphere_i].size() * 12, FoundSphere_VertexBuffers[Sphere_i].size() / 2, nullptr, 0);
			}

			trimodel->isVisible = true;
			trimodel->NAME = "SphereDetection_" + PC->NAME + "_" + to_string(HOUGHSPHERE_BoundingCubeRes) + "_" + 
				to_string(HOUGHSPHERE_MinLocalMaximaThreshold) + "_" + to_string(HOUGHSPHERE_MinSphereRadius);
			trimodel->NonIndexed_VertexBuffers = FoundSphere_VertexPositions;
			trimodel->PATH = "Generated";
			PCViewer::TriangleModel::TriangleModelNormals normallist;
			normallist.NonIndexed_VertexNormals = FoundSphere_VertexNormals;
			normallist.NORMALNAME = "Original";
			trimodel->VertexNormalTYPEs.push_back(normallist);

			Viewer->DisplayableDatas.push_back(trimodel);
		}
		
		for (unsigned int i = 0; i < HOUGH_Spheres.size(); i++) {
			IMGUI->Text(("Sphere " + std::to_string(i) + " X:" + std::to_string(HOUGH_Spheres[i].x) +
				" Y:" + std::to_string(HOUGH_Spheres[i].y) + " Z:" + std::to_string(HOUGH_Spheres[i].z) + " Radius:" + std::to_string(HOUGH_Spheres[i].w)).c_str());
		}

		IMGUI->End_TabItem();
	}
	/*if (IMGUI->Begin_TabItem("Plane Detection")) {
		//Plane Detection Variables
		static int HOUGHPLANE_BoundingCubeRes = 6, HOUGHPLANE_AngleSampleCount = 6, HOUGHPLANE_MaximaThreshold = 10;
		static unsigned int Step = 0;
		static std::vector<GFX_API::SpecialDrawCall> SpecialDrawCalls;
		static std::vector<TuranEditor::Algorithms::Plane> FoundPlanes;
		static std::vector<unsigned int> Plane_GPUHandles;
		static bool ShouldContinue = true, isProgressive = false;
		IMGUI->Slider_Int("Bounding Cube Resolution", &HOUGHPLANE_BoundingCubeRes, 6, 200);
		IMGUI->Slider_Int("Angle Sample Count", &HOUGHPLANE_AngleSampleCount, 2, 180);
		IMGUI->Slider_Int("Maxima Threshold", &HOUGHPLANE_MaximaThreshold, 2, 5000);

		PCViewer::PointCloud_DD* PC = static_cast<PCViewer::PointCloud_DD*>(Viewer->DisplayableDatas[selectedddindex]);
		if (IMGUI->Button("Calculate")) {
			TuranEditor::Algorithms::HoughBasedPlaneDetection(&PC->PC, HOUGHPLANE_BoundingCubeRes, HOUGHPLANE_AngleSampleCount, HOUGHPLANE_MaximaThreshold, FoundPlanes);
			LOG_STATUS("Hough Plane Detection is ended!");
			Plane_GPUHandles.clear();
			for (unsigned int i = 0; i < FoundPlanes.size(); i++) {
				Plane_GPUHandles.push_back(TuranEditor::RenderDataManager::Create_Plane(FoundPlanes[i].Center, FoundPlanes[i].Tangent, FoundPlanes[i].Bitangent, FoundPlanes[i].Size));
			}
		}
		if (IMGUI->Button("Calculate Progressive")) {
			isProgressive = true;
		}
		if (isProgressive) {
			if (IMGUI->Button("Continue")) {
				ShouldContinue = true;
			}
			if (TuranEditor::Algorithms::Progressive_HoughBasedPlaneDetection(&PC->PC, HOUGHPLANE_BoundingCubeRes, HOUGHPLANE_AngleSampleCount, HOUGHPLANE_MaximaThreshold,
				Step, ShouldContinue, SpecialDrawCalls, PC->PCRenderer)) {
				LOG_CRASHING("Progressive Hough Based Plane Detection is finished!");
				isProgressive = false;
			}
			else {
				ShouldContinue = false;
			}
		}
		for (unsigned int Plane_i = 0; Plane_i < Plane_GPUHandles.size(); Plane_i++) {
			GFX_API::DrawCall PlaneDC;
			PlaneDC.JoinedDrawPasses = 0xFFFFFFF;
			PlaneDC.MeshBuffer_ID = Plane_GPUHandles[Plane_i];
			PlaneDC.ShaderInstance_ID = Viewer->SurfaceMaterialInst;
			Viewer->RG->Register_DrawCall(PlaneDC);
		}
		IMGUI->End_TabItem();
	}*/

	IMGUI->End_TabBar();
}