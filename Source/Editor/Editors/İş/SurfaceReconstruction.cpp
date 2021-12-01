#include "PointCloudTools.h"
#include "Editor/TUBITAK/Algorithms.h"

string IMPORTNAME = "";
PCViewer::TriangleModel* lastgeneratedmodel = nullptr;
unsigned int LASTUSED_selectedddindex = UINT32_MAX;
static PCViewer* Viewer = nullptr;
static bool isDebuggingEnabled = true;


void SurfaceRec() {
	IMGUI->Input_Text("Reconstructed Model Name", &IMPORTNAME); 

	static unsigned int selectedlistitemindex = 0, selectedddindex = 0;
	Viewer->SelectListOneLine_FromDisplayableDatas(PCViewer::DisplayableData::POINTCLOUD, selectedlistitemindex, selectedddindex, "Point Cloud");
	if (selectedddindex != UINT32_MAX) {
		PCViewer::PointCloud_DD* pc_dd = static_cast<PCViewer::PointCloud_DD*>(Viewer->DisplayableDatas[selectedddindex]);


		static unsigned int selectedNormalListIndex = 0;
		static int SDFResNaive = 10, SDFResMultiplier = 1, SDFResolution = 10, kNNCount = 2;
		static bool ShowSDFVolume = false;
		static float SamplingD = 1.0f;

		if (pc_dd->PC.PointNormals.size()) {
			vector<const char*> NORMALLIST_names(pc_dd->PC.PointNormals.size());
			for (unsigned int i = 0; i < NORMALLIST_names.size(); i++) {
				NORMALLIST_names[i] = pc_dd->PC.PointNormals[i].NAME.c_str();
			}
			selectedNormalListIndex = std::min(selectedNormalListIndex, unsigned int(NORMALLIST_names.size() - 1));
			IMGUI->SelectList_OneLine("Normal List to Use", &selectedNormalListIndex, NORMALLIST_names);
		}
		else {
			selectedNormalListIndex = UINT32_MAX;
			IMGUI->Text("Point cloud should have normals to calculate H.Hoppe reconstruction");
		}

		static TuranEditor::POINTRENDERER* samplepoint_renderer = nullptr;
		IMGUI->Slider_Int("SDF Resolution", &SDFResNaive, 2, 50);
		IMGUI->Slider_Int("SDF Resolution Multiplier", &SDFResMultiplier, 1, 100);
		IMGUI->Slider_Float("SamplingD", &SamplingD, 1.0 / float(SDFResolution), SDFResNaive);
		SDFResolution = SDFResNaive * SDFResMultiplier;
		IMGUI->Checkbox("Show SDF Volume", &ShowSDFVolume);
		if (IMGUI->Button("Reconstruct") && selectedNormalListIndex != UINT32_MAX) {
			static unsigned int RECONSTRUCTEDMESH_ID = UINT32_MAX;
			TuranEditor::Algorithms::Generate_KDTree(pc_dd->PC);
			vector<vec3> SampleLocations;
			vector<float> SDFValues = TuranEditor::Algorithms::SDF_FromPointCloud(&pc_dd->PC, uvec3(SDFResolution), selectedNormalListIndex, SamplingD, &SampleLocations);
			samplepoint_renderer = TuranEditor::RenderDataManager::Create_PointRenderer(SDFValues.size());
			for (unsigned int i = 0; i < SDFValues.size(); i++) {
				samplepoint_renderer->GetPointPosition_byIndex(i) = SampleLocations[i];
				if (SDFValues[i] == FLT_MAX) { samplepoint_renderer->GetPointCOLORRGBA_byIndex(i) = vec4(0, 0, 1.0, 0.0); }
				else if (SDFValues[i] == -FLT_MAX) { samplepoint_renderer->GetPointCOLORRGBA_byIndex(i) = vec4(0, 1.0, 0.0, 0.0); }
				else { samplepoint_renderer->GetPointCOLORRGBA_byIndex(i) = vec4(vec3((SDFValues[i] + 1.0) / 2.0), 1.0); }
			}
			vector<vec3> VertexNormals;
			vector<vec3> VertexPositions = TuranEditor::Algorithms::MarchingCubes(uvec3(SDFResolution), SDFValues, SampleLocations, &pc_dd->PC, &VertexNormals);
			vector<vec3> VertexBuffer(VertexPositions.size() * 2, vec3(0.0));
			memcpy(VertexBuffer.data(), VertexPositions.data(), VertexPositions.size() * 12);
			memcpy(VertexBuffer.data() + VertexPositions.size(), VertexNormals.data(), VertexNormals.size() * 12);

			RECONSTRUCTEDMESH_ID = GFXContentManager->Upload_MeshBuffer(TuranEditor::RenderDataManager::PositionNormal_VertexAttrib, VertexBuffer.data(), VertexBuffer.size() * 12, VertexPositions.size(), nullptr, 0);

			PCViewer::TriangleModel* trimodel = new PCViewer::TriangleModel;
			trimodel->NAME = IMPORTNAME + to_string(SDFResolution) + "x" + to_string(SDFResolution) + "-e" + to_string(SamplingD);
			trimodel->PATH = "Generated";
			trimodel->isVisible = true;
			trimodel->GPUMESH_IDs.push_back(RECONSTRUCTEDMESH_ID);
			trimodel->DisplayedVertexBuffers.push_back(true);
			trimodel->NonIndexed_VertexBuffers.push_back(VertexPositions);
			PCViewer::TriangleModel::TriangleModelNormals normallistobj;
			normallistobj.NORMALNAME = "MC_Normals";
			normallistobj.NonIndexed_VertexNormals.push_back(VertexNormals);
			trimodel->VertexNormalTYPEs.push_back(normallistobj);
			trimodel->BOUNDINGMAX = SampleLocations[SampleLocations.size() - 1];
			trimodel->BOUNDINGMIN = SampleLocations[0];
			trimodel->BoundingSphereRadius = length(trimodel->BOUNDINGMAX - trimodel->BOUNDINGMIN) / 2.0;
			trimodel->CenterOfData = trimodel->BOUNDINGMIN + ((trimodel->BOUNDINGMAX - trimodel->BOUNDINGMIN) / dvec3(2.0));
			lastgeneratedmodel = trimodel;
			LASTUSED_selectedddindex = selectedddindex;

			

			Viewer->DisplayableDatas.push_back(trimodel);
		}
		/*
		if (lastgeneratedmodel && LASTUSED_selectedddindex == selectedddindex) {
			static int NewSDFMultiplier = 0; static float newSamplingD = 1.0f;
			NewSDFMultiplier = std::max(NewSDFMultiplier, SDFResMultiplier);
			IMGUI->Slider_Int("New Multiplier", &NewSDFMultiplier, SDFResMultiplier, SDFResMultiplier * 2);
			IMGUI->Slider_Float("New SamplingD", &newSamplingD, 1.0 / float(SDFResolution), SDFResNaive);
			if (IMGUI->Button("Smooth")) {
				vector<vec3> SamplePositions;
				vector<float> SDFs = TuranEditor::Algorithms::SDF_FromVertexBuffer(lastgeneratedmodel->NonIndexed_VertexBuffers[0], lastgeneratedmodel->VertexNormalsList[0].NonIndexed_VertexNormals[0],
					uvec3(SDFResNaive * NewSDFMultiplier), pc_dd->CenterOfData - dvec3(pc_dd->BoundingSphereRadius), pc_dd->CenterOfData + dvec3(pc_dd->BoundingSphereRadius), newSamplingD, &SamplePositions);

				vector<vec3> VertexBuffer = TuranEditor::Algorithms::MarchingCubes(uvec3(SDFResNaive * NewSDFMultiplier), SDFs, SamplePositions, nullptr);

				unsigned int RECONSTRUCTEDMESH_ID = GFXContentManager->Upload_MeshBuffer(TuranEditor::RenderDataManager::PositionOnly_VertexAttrib, VertexBuffer.data(), VertexBuffer.size() * 12, VertexBuffer.size(), nullptr, 0);

				PCViewer::TriangleModel* trimodel = new PCViewer::TriangleModel;
				trimodel->NAME = lastgeneratedmodel->NAME + "_Smoother" + to_string(NewSDFMultiplier);
				trimodel->PATH = "Generated";
				trimodel->isVisible = true;
				trimodel->GPUMESH_IDs.push_back(RECONSTRUCTEDMESH_ID);
				trimodel->DisplayedVertexBuffers.push_back(true);
				trimodel->NonIndexed_VertexBuffers.push_back(VertexBuffer);
				lastgeneratedmodel = trimodel;
				LASTUSED_selectedddindex = selectedddindex;

				Viewer->DisplayableDatas.push_back(trimodel);
			}
		}*/
		if (ShowSDFVolume && samplepoint_renderer) {
			samplepoint_renderer->RenderThisFrame();
		}
	}
}

void SurfaceRecDebugging() {
	static ivec3 CubeCount(2); static bool isSDFCountUpdated = true, isCubeSizeLocorAnySDF_updated = true;
	if (IMGUI->Slider_Int("Cube Count X", &CubeCount.x, 2, 5)) { isSDFCountUpdated = true; }
	if (IMGUI->Slider_Int("Cube Count Y", &CubeCount.y, 2, 5)) { isSDFCountUpdated = true; }
	if (IMGUI->Slider_Int("Cube Count Z", &CubeCount.z, 2, 5)) { isSDFCountUpdated = true; }
	static vec3 CubeSize = vec3(1.0f), FirstCubeLocation = vec3(0);
	float MaxUSDFValue = length(CubeSize);
	
	static vector<float> SDFs; static TuranEditor::POINTRENDERER* samplepoint_renderer = nullptr;
	//Resize SDFs buffer
	if (isSDFCountUpdated = true) {
		SDFs.resize((CubeCount.x + 1) * (CubeCount.y + 1) * (CubeCount.z + 1), -MaxUSDFValue);

		TuranEditor::RenderDataManager::DestroyPointRenderer(samplepoint_renderer);
		samplepoint_renderer = TuranEditor::RenderDataManager::Create_PointRenderer(SDFs.size());

		for (unsigned int i = 0; i < SDFs.size(); i++) {
			unsigned int CubeIndexZ = i / ((CubeCount.x + 1) * (CubeCount.y + 1));
			unsigned int CubeIndexY = (i - (CubeIndexZ * (CubeCount.x + 1) * (CubeCount.y + 1))) / (CubeCount.x + 1);
			unsigned int CubeIndexX = i - (CubeIndexZ * (CubeCount.x + 1) * (CubeCount.y + 1)) - (CubeIndexY * (CubeCount.y + 1));
			samplepoint_renderer->GetPointPosition_byIndex(i) = FirstCubeLocation + vec3(CubeSize.x * CubeIndexX, CubeSize.y * CubeIndexY, CubeSize.z * CubeIndexZ);
			samplepoint_renderer->GetPointCOLORRGBA_byIndex(i) = vec4(vec3((SDFs[i] + MaxUSDFValue) / (MaxUSDFValue * 2)), 1.0f);
		}

		isSDFCountUpdated = false;
	}

	if (IMGUI->Slider_Vec3("Cube Size", &CubeSize, 1.0f / length(vec3(CubeCount)), length(vec3(CubeCount)) * length(vec3(CubeCount)))) { isCubeSizeLocorAnySDF_updated = true; }
	if (IMGUI->Slider_Vec3("First Cube Location", &FirstCubeLocation, -100.0, 100.0)) { isCubeSizeLocorAnySDF_updated = true; }
	static unsigned int ModifiedCubeIndex = 0;
	ModifiedCubeIndex = std::min(ModifiedCubeIndex, unsigned int(CubeCount.x * CubeCount.y * CubeCount.z) - 1);
	vector<string> CubeNamesList(CubeCount.x * CubeCount.y * CubeCount.z, "nan");
	for (unsigned int i = 0; i < CubeNamesList.size(); i++) { CubeNamesList[i] = to_string(i); }
	IMGUI->SelectList_OneLine("Cube to modify", &ModifiedCubeIndex, &CubeNamesList);

	static PCViewer::TriangleModel* trimodel = nullptr;
	{
		unsigned int ModifiedCubeZIndex = ModifiedCubeIndex / (CubeCount.y * CubeCount.x);
		unsigned int ModifiedCubeYIndex = (ModifiedCubeIndex - (ModifiedCubeZIndex * CubeCount.x * CubeCount.y)) / (CubeCount.x);
		unsigned int ModifiedCubeXIndex = ModifiedCubeIndex - (ModifiedCubeZIndex * CubeCount.x * CubeCount.y) - (ModifiedCubeYIndex * CubeCount.x);
		unsigned int SDFIndexes[8] = { (ModifiedCubeZIndex * (CubeCount.x + 1) * (CubeCount.y + 1)) + (ModifiedCubeYIndex * (CubeCount.x + 1)) + ModifiedCubeXIndex,
		(ModifiedCubeZIndex * (CubeCount.x + 1) * (CubeCount.y + 1)) + (ModifiedCubeYIndex * (CubeCount.x + 1)) + ModifiedCubeXIndex + 1,
		((ModifiedCubeZIndex + 1) * (CubeCount.x + 1) * (CubeCount.y + 1)) + (ModifiedCubeYIndex * (CubeCount.x + 1)) + ModifiedCubeXIndex + 1,
		((ModifiedCubeZIndex + 1) * (CubeCount.x + 1) * (CubeCount.y + 1)) + (ModifiedCubeYIndex * (CubeCount.x + 1)) + ModifiedCubeXIndex,
		(ModifiedCubeZIndex * (CubeCount.x + 1) * (CubeCount.y + 1)) + ((ModifiedCubeYIndex + 1) * (CubeCount.x + 1)) + ModifiedCubeXIndex,
		(ModifiedCubeZIndex * (CubeCount.x + 1) * (CubeCount.y + 1)) + ((ModifiedCubeYIndex + 1) * (CubeCount.x + 1)) + ModifiedCubeXIndex + 1,
		((ModifiedCubeZIndex + 1) * (CubeCount.x + 1) * (CubeCount.y + 1)) + ((ModifiedCubeYIndex + 1) * (CubeCount.x + 1)) + ModifiedCubeXIndex + 1,
		((ModifiedCubeZIndex + 1) * (CubeCount.x + 1) * (CubeCount.y + 1)) + ((ModifiedCubeYIndex + 1) * (CubeCount.x + 1)) + ModifiedCubeXIndex
		};


		vec4 bottom = vec4(SDFs[SDFIndexes[0]], SDFs[SDFIndexes[1]], SDFs[SDFIndexes[2]], SDFs[SDFIndexes[3]]), 
			top = vec4(SDFs[SDFIndexes[4]], SDFs[SDFIndexes[5]], SDFs[SDFIndexes[6]], SDFs[SDFIndexes[7]]);
		if (IMGUI->Slider_Vec4("Bottom", &bottom, -MaxUSDFValue, MaxUSDFValue)) { isCubeSizeLocorAnySDF_updated = true; }
		if (IMGUI->Slider_Vec4("Top", &top, -MaxUSDFValue, MaxUSDFValue)) { isCubeSizeLocorAnySDF_updated = true; }
		SDFs[SDFIndexes[0]] = bottom.x; SDFs[SDFIndexes[1]] = bottom.y; SDFs[SDFIndexes[2]] = bottom.z; SDFs[SDFIndexes[3]] = bottom.w;
		SDFs[SDFIndexes[4]] = top.x; SDFs[SDFIndexes[5]] = top.y; SDFs[SDFIndexes[6]] = top.z; SDFs[SDFIndexes[7]] = top.w;
	}

	if (isCubeSizeLocorAnySDF_updated) {
		if (trimodel) {
			GFXContentManager->Unload_MeshBuffer(trimodel->GPUMESH_IDs[0]); delete trimodel;
			for (unsigned int i = 0; i < Viewer->DisplayableDatas.size(); i++) if (trimodel == Viewer->DisplayableDatas[i]) 
			{ Viewer->DisplayableDatas.erase(Viewer->DisplayableDatas.begin() + i); }
		}

		vector<vec3> SamplePositions(SDFs.size(), vec3(0.0));
		for (unsigned int i = 0; i < SDFs.size(); i++) {
			unsigned int CubeIndexZ = i / ((CubeCount.x + 1) * (CubeCount.y + 1));
			unsigned int CubeIndexY = (i - (CubeIndexZ * (CubeCount.x + 1) * (CubeCount.y + 1))) / (CubeCount.x + 1);
			unsigned int CubeIndexX = i - (CubeIndexZ * (CubeCount.x + 1) * (CubeCount.y + 1)) - (CubeIndexY * (CubeCount.y + 1));
			SamplePositions[i] = FirstCubeLocation + vec3(CubeSize.x * CubeIndexX, CubeSize.y * CubeIndexY, CubeSize.z * CubeIndexZ);
			samplepoint_renderer->GetPointPosition_byIndex(i) = SamplePositions[i];
			samplepoint_renderer->GetPointCOLORRGBA_byIndex(i) = vec4(vec3((SDFs[i] + MaxUSDFValue) / (MaxUSDFValue * 2)), 1.0f);
		}

		trimodel = new PCViewer::TriangleModel;
		trimodel->NAME = "RecDebugging_" + to_string(CubeCount.x) + "x" + to_string(CubeCount.y) + "x" + to_string(CubeCount.z);
		trimodel->PATH = "Generated";
		trimodel->isVisible = true;
		trimodel->BOUNDINGMAX = FirstCubeLocation + (CubeSize * vec3(CubeCount));
		trimodel->BOUNDINGMIN = FirstCubeLocation;
		trimodel->BoundingSphereRadius = length(CubeSize * vec3(CubeCount)) / 2.0;
		trimodel->CenterOfData = FirstCubeLocation + (CubeSize * vec3(CubeCount) / vec3(2.0));
		vector<vec3> VertexBuffer = TuranEditor::Algorithms::MarchingCubes(CubeCount + 1, SDFs, SamplePositions, nullptr);
		trimodel->GPUMESH_IDs.push_back(GFXContentManager->Upload_MeshBuffer(TuranEditor::RenderDataManager::PositionNormal_VertexAttrib
			, VertexBuffer.data(), VertexBuffer.size() * 12, VertexBuffer.size(), nullptr, 0));
		trimodel->DisplayedVertexBuffers.push_back(true);
		trimodel->NonIndexed_VertexBuffers.push_back(VertexBuffer);
		Viewer->DisplayableDatas.push_back(trimodel);


		isCubeSizeLocorAnySDF_updated = false;
	}
	samplepoint_renderer->RenderThisFrame();
}

void PointCloudTools::SurfaceReconstruction(PCViewer* viewer) {
	Viewer = viewer;

	IMGUI->Checkbox("Debugging enabled", &isDebuggingEnabled);
	if (isDebuggingEnabled) {
		if (!IMGUI->Begin_TabBar()) { return; }
		if (IMGUI->Begin_TabItem("Surface Reconstruction")) { SurfaceRec(); IMGUI->End_TabItem(); }
		if (IMGUI->Begin_TabItem("Debugging")) { SurfaceRecDebugging(); IMGUI->End_TabItem(); }
		IMGUI->End_TabBar();
	}
	else {
		SurfaceRec();
	}
}