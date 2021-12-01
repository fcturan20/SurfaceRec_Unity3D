#include "PointCloudTools.h"
#include "Editor/TUBITAK/Algorithms.h"
#include "Editor/RenderContext/Draw Passes/GBuffer_DrawPass.h"

static string IMPORTNAME;

static PCViewer* viewer = nullptr;
struct ProgressivePointCloudGenerator {
	vector<vec3> PointList;
	vector<unsigned int> Point_to_CameraPos_IndirectionTable;
	vector<vec3> CameraPosList;
};
static ProgressivePointCloudGenerator* generator = nullptr;
static TuranEditor::POINTRENDERER* generatorrenderer = nullptr;


void RayTraceandGetPoints(vector<vec3>& Points) {
	vector<float> DATA;
	unsigned int DATASIZE = 0;
	void* DepthBuffer = viewer->RG->GetDepthBuffer(DATASIZE);
	DATA.resize(DATASIZE / 4);
	memcpy(DATA.data(), DepthBuffer, DATASIZE);
	delete DepthBuffer;

	if (DATA.size() == 1920 * 1080) {
		for (unsigned int PixelIndex = 0; PixelIndex < DATA.size(); PixelIndex++) {
			if (DATA[PixelIndex] == 1.0) { continue; }
			unsigned int PixelY = PixelIndex / 1920;
			unsigned int PixelX = PixelIndex - (1920 * PixelY);
			vec2 TextCoord = vec2(float(PixelX) / 1920.0, float(PixelY) / 1080.0);

			//Depth Buffer to World Space
			{
				float z = DATA[PixelIndex] * 2.0 - 1.0;

				vec4 clipSpacePosition = vec4((TextCoord * vec2(2.0)) - vec2(1.0), z, 1.0);
				vec4 viewSpacePosition = inverse(TuranEditor::RenderDataManager::CAMERABUFFER_DATA[0]) * clipSpacePosition;

				// Perspective division
				viewSpacePosition /= viewSpacePosition.w;

				vec4 worldSpacePosition = inverse(TuranEditor::RenderDataManager::CAMERABUFFER_DATA[1]) * viewSpacePosition;

				Points.push_back(worldSpacePosition);
			}
		}
	}
	else {
		LOG_CRASHING("Ray tracing is only done for 1920 * 1080 resolution!");
	}

}

void ModelSmoother();


void PointCloudTools::CoreAlgorithms(PCViewer* Viewer) {
	viewer = Viewer;
	if (!IMGUI->Begin_TabBar()) {
		return;
	}

	if (IMGUI->Begin_TabItem("Voronoi")) {
		IMGUI->Input_Text("Voronoi Diagram's Name", &IMPORTNAME);
		static unsigned int selectedlistitemindex = 0, selectedddindex = 0;
		Viewer->SelectListOneLine_FromDisplayableDatas(PCViewer::DisplayableData::DataType::POINTCLOUD, selectedlistitemindex, selectedddindex, "Point Cloud to Calculate Voronoi");
		if (selectedddindex != UINT32_MAX) {
			PCViewer::PointCloud_DD* pc_dd = static_cast<PCViewer::PointCloud_DD*>(Viewer->DisplayableDatas[selectedddindex]);
			if (pc_dd->PC.PointNormals.size()) {
				vector<const char*> NORMALLIST_names(pc_dd->PC.PointNormals.size());
				for (unsigned int i = 0; i < NORMALLIST_names.size(); i++) {
					NORMALLIST_names[i] = pc_dd->PC.PointNormals[i].NAME.c_str();
				}
				static unsigned int selectedNormalListIndex = 0;
				selectedNormalListIndex = std::min(selectedNormalListIndex, unsigned int(NORMALLIST_names.size() - 1));
				IMGUI->SelectList_OneLine("Normal List to Calculate Voronoi With", &selectedNormalListIndex, NORMALLIST_names);
				IMGUI->Same_Line();
			}

			static TuranEditor::Algorithms::VoronoiDiagram* Diagram;
			//VoronoiVisualizerIndex 0 means point renderer, 1 means region edge renderer
			//ActiveVisualizedDataIndex means the index of the cluster that's being shown. UINT32_MAX means no visualizations or index isn't used.
			static unsigned int VoronoiVisualizerIndex = UINT32_MAX, ActiveVisualizedDataIndex = UINT32_MAX;
			if (IMGUI->Button("Calculate Voronoi Diagram")) {
				Diagram = TuranEditor::Algorithms::CreateVoronoiDiagram(&static_cast<PCViewer::PointCloud_DD*>(Viewer->DisplayableDatas[selectedddindex])->PC);
				LOG_STATUS("Voronoi is calculated\n\n\n");

				//Voronoi Vertices Point Cloud
				{
					PCViewer::PointCloud_DD* VoronoiVerticesPC = new PCViewer::PointCloud_DD;
					VoronoiVerticesPC->isVisible = true;
					VoronoiVerticesPC->NAME = IMPORTNAME + " VoronoiVertices PC";
					VoronoiVerticesPC->PATH = "Generated";
					VoronoiVerticesPC->PCRenderer = TuranEditor::RenderDataManager::Create_PointRenderer(Diagram->VoronoiVertices.size());
					VoronoiVerticesPC->PC.PointCount = Diagram->VoronoiVertices.size();
					VoronoiVerticesPC->PC.PointPositions = new vec3[VoronoiVerticesPC->PC.PointCount];

					for (unsigned int i = 0; i < Diagram->VoronoiVertices.size(); i++) {
						VoronoiVerticesPC->PCRenderer->GetPointCOLORRGBA_byIndex(i) = vec4(1.0f);
						VoronoiVerticesPC->PCRenderer->GetPointPosition_byIndex(i) = Diagram->VoronoiVertices[i];
						VoronoiVerticesPC->PC.PointPositions[i] = Diagram->VoronoiVertices[i];
					}

					for (unsigned int i = 0; i < VoronoiVerticesPC->PC.PointCount; i++) {
						VoronoiVerticesPC->CenterOfData += VoronoiVerticesPC->PC.PointPositions[i];
					}
					VoronoiVerticesPC->CenterOfData = VoronoiVerticesPC->CenterOfData / dvec3(VoronoiVerticesPC->PC.PointCount);
					for (unsigned int i = 0; i < VoronoiVerticesPC->PC.PointCount; i++) {
						double dist = length(VoronoiVerticesPC->CenterOfData - dvec3(VoronoiVerticesPC->PC.PointPositions[i]));
						if (dist > VoronoiVerticesPC->BoundingSphereRadius) { VoronoiVerticesPC->BoundingSphereRadius = dist; }
					}

					Viewer->DisplayableDatas.push_back(VoronoiVerticesPC);
				}

				//Voronoi Cells Static Model
				{
					PCViewer::TriangleModel* VD_MODEL = new PCViewer::TriangleModel;
					VD_MODEL->DisplayedVertexBuffers.resize(Diagram->VoronoiRegions.size());
					VD_MODEL->GPUMESH_IDs.resize(Diagram->VoronoiRegions.size());
					VD_MODEL->isVisible = true;
					VD_MODEL->NAME = IMPORTNAME + " VoronoiCells";
					VD_MODEL->NonIndexed_VertexBuffers.resize(Diagram->VoronoiRegions.size());
					VD_MODEL->PATH = "Generated";

					for (unsigned int VorCellIndex = 0; VorCellIndex < Diagram->VoronoiRegions.size(); VorCellIndex++) {
						std::vector<vec3> Buf(Diagram->VoronoiRegions[VorCellIndex].TriangleIDs.size() * 3);
						for (unsigned int TriangleIndex = 0; TriangleIndex < Diagram->VoronoiRegions[VorCellIndex].TriangleIDs.size(); TriangleIndex++) {
							Buf[TriangleIndex * 3] = Diagram->VoronoiTriangles[Diagram->VoronoiRegions[VorCellIndex].TriangleIDs[TriangleIndex]].Vertexes[0];
							Buf[(TriangleIndex * 3) + 1] = Diagram->VoronoiTriangles[Diagram->VoronoiRegions[VorCellIndex].TriangleIDs[TriangleIndex]].Vertexes[1];
							Buf[(TriangleIndex * 3) + 2] = Diagram->VoronoiTriangles[Diagram->VoronoiRegions[VorCellIndex].TriangleIDs[TriangleIndex]].Vertexes[2];
						}
						VD_MODEL->GPUMESH_IDs[VorCellIndex] = GFXContentManager->Upload_MeshBuffer(TuranEditor::RenderDataManager::PositionOnly_VertexAttrib, Buf.data(), Buf.size() * 12, Buf.size(), nullptr, 0);
						VD_MODEL->NonIndexed_VertexBuffers[VorCellIndex] = Buf;
						VD_MODEL->DisplayedVertexBuffers[VorCellIndex] = false;
					}

					unsigned int TotalVertexCount = 0;
					for (unsigned int MeshIndex = 0; MeshIndex < VD_MODEL->NonIndexed_VertexBuffers.size(); MeshIndex++) {
						for (unsigned int i = 0; i < VD_MODEL->NonIndexed_VertexBuffers[MeshIndex].size(); i++) {
							VD_MODEL->CenterOfData += VD_MODEL->NonIndexed_VertexBuffers[MeshIndex][i];
						}
						TotalVertexCount += VD_MODEL->NonIndexed_VertexBuffers[MeshIndex].size();
					}

					VD_MODEL->CenterOfData = VD_MODEL->CenterOfData / dvec3(TotalVertexCount);
					for (unsigned int MeshIndex = 0; MeshIndex < VD_MODEL->NonIndexed_VertexBuffers.size(); MeshIndex++) {
						for (unsigned int i = 0; i < VD_MODEL->NonIndexed_VertexBuffers[MeshIndex].size(); i++) {
							double dist = length(VD_MODEL->CenterOfData - dvec3(VD_MODEL->NonIndexed_VertexBuffers[MeshIndex][i]));
							if (dist > VD_MODEL->BoundingSphereRadius) { VD_MODEL->BoundingSphereRadius = dist; }
						}
					}
					Viewer->DisplayableDatas.push_back(VD_MODEL);
				}
			}
			else {
				IMGUI->Text("Voronoi diagram isn't calculated, so nothing to show here");
			}
		}
		else {
			IMGUI->Text("Please provide a point cloud!");
		}
		IMGUI->End_TabItem();
	}
	if (IMGUI->Begin_TabItem("Ray Tracing")) {
		//Create progressive generator
		if (!generator) {
			IMGUI->Input_Text("Imported points' name", &IMPORTNAME);
			if (IMGUI->Button("Import Points")) {
				vector<vec3> Points;
				RayTraceandGetPoints(Points);

				if (Points.size()) {
					generator = new ProgressivePointCloudGenerator;

					generator->PointList.insert(generator->PointList.end(), Points.begin(), Points.end());
					generator->CameraPosList.push_back(TuranEditor::RenderDataManager::CameraPos);
					generator->Point_to_CameraPos_IndirectionTable.push_back(generator->PointList.size());
				}

				TuranEditor::RenderDataManager::DestroyPointRenderer(generatorrenderer);
				generatorrenderer = TuranEditor::RenderDataManager::Create_PointRenderer(generator->PointList.size());
				unsigned int PointIndex = 0;
				for (unsigned int CameraIndex = 0; CameraIndex < generator->CameraPosList.size(); CameraIndex++) {
					vec4 CameraColor = vec4(static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
						static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
						static_cast <float> (rand()) / static_cast <float> (RAND_MAX), 1.0f);
					for (PointIndex; PointIndex < generator->Point_to_CameraPos_IndirectionTable[CameraIndex]; PointIndex++) {
						generatorrenderer->GetPointPosition_byIndex(PointIndex) = generator->PointList[PointIndex];
						generatorrenderer->GetPointCOLORRGBA_byIndex(PointIndex) = CameraColor;
					}
				}
			}
		}
		else {
			if (IMGUI->Button("Generate new points")) {
				vector<vec3> NewPoints;
				RayTraceandGetPoints(NewPoints);

				generator->PointList.insert(generator->PointList.end(), NewPoints.begin(), NewPoints.end());
				generator->CameraPosList.push_back(TuranEditor::RenderDataManager::CameraPos);
				generator->Point_to_CameraPos_IndirectionTable.push_back(generator->PointList.size());

				TuranEditor::RenderDataManager::DestroyPointRenderer(generatorrenderer);
				generatorrenderer = TuranEditor::RenderDataManager::Create_PointRenderer(generator->PointList.size());
				unsigned int PointIndex = 0;
				for (unsigned int CameraIndex = 0; CameraIndex < generator->CameraPosList.size(); CameraIndex++) {
					vec4 CameraColor = vec4(static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
						static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
						static_cast <float> (rand()) / static_cast <float> (RAND_MAX), 1.0f);
					for (PointIndex; PointIndex < generator->Point_to_CameraPos_IndirectionTable[CameraIndex]; PointIndex++) {
						generatorrenderer->GetPointPosition_byIndex(PointIndex) = generator->PointList[PointIndex];
						generatorrenderer->GetPointCOLORRGBA_byIndex(PointIndex) = CameraColor;
					}
				}
			}

			static int kNNCount = 2;
			IMGUI->Slider_Int("kNN Count for PCA", &kNNCount, 2, 100);
			if (IMGUI->Button("Finish")) {
				PCViewer::PointCloud_DD* ImportedPC = new PCViewer::PointCloud_DD;

				ImportedPC->NAME = IMPORTNAME;
				ImportedPC->PATH = "Generated";
				ImportedPC->isVisible = true;
				ImportedPC->TYPE = PCViewer::PointCloud_DD::POINTCLOUD;

				ImportedPC->PC.PointCount = generator->PointList.size();
				ImportedPC->PC.PointPositions = new vec3[ImportedPC->PC.PointCount]{ vec3(0) };
				for (unsigned int i = 0; i < ImportedPC->PC.PointCount; i++) {
					ImportedPC->PC.PointPositions[i] = generator->PointList[i];
				}

				//Calculate PCA
				TuranEditor::Algorithms::Generate_KDTree(ImportedPC->PC);
				PC_PointNormals pcaresult;
				pcaresult.NAME = "RT_PCA " + to_string(kNNCount);
				pcaresult.Normals = new vec3[ImportedPC->PC.PointCount];
				unsigned int PointIndex = 0;
				for (unsigned int CameraIndex = 0; CameraIndex < generator->CameraPosList.size(); CameraIndex++) {
					for (PointIndex; PointIndex < generator->Point_to_CameraPos_IndirectionTable[CameraIndex]; PointIndex++) {
						vector<unsigned int> kNN_indexes = TuranEditor::Algorithms::Searchfor_ClosestNeighbors(ImportedPC->PC, ImportedPC->PC.PointPositions[PointIndex], kNNCount);
						vector<vec3> kNNPositions(kNN_indexes.size());
						for (unsigned int knnIndex = 0; knnIndex < kNN_indexes.size(); knnIndex++) {
							kNNPositions[knnIndex] = ImportedPC->PC.PointPositions[kNN_indexes[knnIndex]];
						}

						vec3 Normal = normalize(TuranEditor::Algorithms::Compute_PCA(kNNPositions)[2]);
						pcaresult.Normals[PointIndex] = Normal * sign(dot(Normal, 
							normalize(generator->CameraPosList[CameraIndex] - ImportedPC->PC.PointPositions[PointIndex])
						));
					}
				}

				ImportedPC->PC.PointNormals.push_back(pcaresult);

				//Calculate the center and the bounding sphere radius
				for (unsigned int i = 0; i < ImportedPC->PC.PointCount; i++) {
					ImportedPC->CenterOfData += ImportedPC->PC.PointPositions[i];
				}
				ImportedPC->CenterOfData = ImportedPC->CenterOfData / dvec3(ImportedPC->PC.PointCount);
				for (unsigned int i = 0; i < ImportedPC->PC.PointCount; i++) {
					double dist = length(ImportedPC->CenterOfData - dvec3(ImportedPC->PC.PointPositions[i]));
					if (dist > ImportedPC->BoundingSphereRadius) { ImportedPC->BoundingSphereRadius = dist; }
				}

				ImportedPC->PCRenderer = TuranEditor::RenderDataManager::Create_PointRenderer(ImportedPC->PC.PointCount);
				if (ImportedPC->PCRenderer == nullptr) {
					LOG_CRASHING("Creating the point buffer on GPU has failed!");
				}
				for (unsigned int i = 0; i < ImportedPC->PC.PointCount; i++) {
					ImportedPC->PCRenderer->GetPointPosition_byIndex(i) = ImportedPC->PC.PointPositions[i];
					ImportedPC->PCRenderer->GetPointCOLORRGBA_byIndex(i) = vec4(1.0f, 0.0f, 0.0f, 1.0f);
				}

				Viewer->DisplayableDatas.push_back(ImportedPC);


				//Destroy temporary data
				delete generator;
				generator = nullptr;
				TuranEditor::RenderDataManager::DestroyPointRenderer(generatorrenderer);
				generatorrenderer = nullptr;
			}
		}
		if (generatorrenderer) {
			generatorrenderer->RenderThisFrame();
		}
		IMGUI->End_TabItem();
	}
	//If generator is created but menu is swapped, delete all generator content
	else if (generator) {
		delete generator;
		generator = nullptr;
		TuranEditor::RenderDataManager::DestroyPointRenderer(generatorrenderer);
		generatorrenderer = nullptr;
	}
	if (IMGUI->Begin_TabItem("Model Smoother")) {
		ModelSmoother();
		IMGUI->End_TabItem();
	}

	IMGUI->End_TabBar();
}


void ModelSmoother() {
	IMGUI->Input_Text("Smooth Model Name", &IMPORTNAME); 
	static unsigned int tri_selectedlistitemindex = 0, tri_selectedddindex = 0, pc_selectednormallistindex = 0, selectedlistitemindex_refpc = 0, selectedddindex_refpc = 0;
	static float SamplingD = 1.0f;
	viewer->SelectListOneLine_FromDisplayableDatas(PCViewer::DisplayableData::TRIANGLEMODEL, tri_selectedlistitemindex, tri_selectedddindex, "Model to Smooth");

	if (tri_selectedddindex == UINT32_MAX) {IMGUI->Text("There is no model to smooth!");	return;}

	PCViewer::TriangleModel* trimodel = nullptr;
	trimodel = static_cast<PCViewer::TriangleModel*>(viewer->DisplayableDatas[tri_selectedddindex]);
	static int SDFResNaive = 10, SDFResMultiplier = 1, SDFResolution = 10;
	static bool ShowSDFVolume = false, ShouldUseRefPC = false;


	if (trimodel->NonIndexed_VertexBuffers.size() > 1) { IMGUI->Text("Models that has multiple meshes aren't supported for now!"); return; }
	if (!trimodel->VertexNormalTYPEs.size()) { IMGUI->Text("There is no vertex normal to use while smoothing!"); return; }


	static unsigned int selectedNormalListIndex = 0;
	vector<const char*> NORMALLIST_names(trimodel->VertexNormalTYPEs.size());
	for (unsigned int i = 0; i < NORMALLIST_names.size(); i++) {
		NORMALLIST_names[i] = trimodel->VertexNormalTYPEs[i].NORMALNAME.c_str();
	}
	selectedNormalListIndex = std::min(selectedNormalListIndex, unsigned int(NORMALLIST_names.size() - 1));
	IMGUI->SelectList_OneLine("Model Normal List", &selectedNormalListIndex, NORMALLIST_names);	

	static TuranEditor::POINTRENDERER* samplepoint_renderer = nullptr;
	IMGUI->Slider_Int("SDF Resolution", &SDFResNaive, 2, 50);
	IMGUI->Slider_Int("SDF Resolution Multiplier", &SDFResMultiplier, 1, 100);
	IMGUI->Checkbox("Show SDF Volume", &ShowSDFVolume);
	IMGUI->Checkbox("Should use reference PC", &ShouldUseRefPC);
	static PCViewer::PointCloud_DD* refpc = nullptr;
	if (ShouldUseRefPC) {
		viewer->SelectListOneLine_FromDisplayableDatas(PCViewer::DisplayableData::POINTCLOUD, selectedlistitemindex_refpc, selectedddindex_refpc, "Reference PC");
		if (selectedddindex_refpc != UINT32_MAX) {
			refpc = static_cast<PCViewer::PointCloud_DD*>(viewer->DisplayableDatas[selectedddindex_refpc]);
		}
		vector<const char*> pc_NORMALLIST_names(refpc->PC.PointNormals.size());
		for (unsigned int i = 0; i < pc_NORMALLIST_names.size(); i++) {
			pc_NORMALLIST_names[i] = refpc->PC.PointNormals[i].NAME.c_str();
		}
		pc_selectednormallistindex = std::min(pc_selectednormallistindex, unsigned int(pc_NORMALLIST_names.size() - 1));
		IMGUI->SelectList_OneLine("Ref PC Normal List", &pc_selectednormallistindex, pc_NORMALLIST_names);
		IMGUI->Slider_Float("SamplingD", &SamplingD, 1.0 / float(SDFResolution), SDFResNaive);
	}
	else { refpc = nullptr; }

	if (IMGUI->Button("Smooth")) {
		trimodel = static_cast<PCViewer::TriangleModel*>(viewer->DisplayableDatas[tri_selectedddindex]);
		vector<vec3> SamplePositions;
		if(refpc){ TuranEditor::Algorithms::Generate_KDTree(refpc->PC); }
		vector<float> SDFs = TuranEditor::Algorithms::SDF_FromVertexBuffer(trimodel->NonIndexed_VertexBuffers[0],
			trimodel->VertexNormalTYPEs[selectedNormalListIndex].NonIndexed_VertexNormals[0], 
			uvec3(SDFResNaive * SDFResMultiplier), trimodel->BOUNDINGMIN, trimodel->BOUNDINGMAX, &SamplePositions, 
			(refpc != nullptr) ? &refpc->PC : nullptr, pc_selectednormallistindex, SamplingD);

		samplepoint_renderer = TuranEditor::RenderDataManager::Create_PointRenderer(SDFs.size());
		for (unsigned int i = 0; i < SDFs.size(); i++) {
			samplepoint_renderer->GetPointPosition_byIndex(i) = SamplePositions[i];
			if (SDFs[i] == FLT_MAX) { samplepoint_renderer->GetPointCOLORRGBA_byIndex(i) = vec4(0, 0, 1.0, 1.0); }
			else if (SDFs[i] == -FLT_MAX) { samplepoint_renderer->GetPointCOLORRGBA_byIndex(i) = vec4(0, 1.0, 0.0, 1.0); }
			else { samplepoint_renderer->GetPointCOLORRGBA_byIndex(i) = vec4(vec3((SDFs[i] + 1.0) / 2.0), 1.0); }
		}

		vector<vec3> VertexNormals;
		vector<vec3> VertexPositions = TuranEditor::Algorithms::MarchingCubes(uvec3(SDFResNaive * SDFResMultiplier), SDFs, SamplePositions, (refpc != nullptr) ? &refpc->PC : nullptr, &VertexNormals);
		
		vector<vec3> VertexBuffer(VertexPositions.size() * 2, vec3(UINT32_MAX));
		memcpy(VertexBuffer.data(), VertexPositions.data(), VertexPositions.size() * 12);
		memcpy(VertexBuffer.data() + VertexPositions.size(), VertexNormals.data(), VertexNormals.size() * 12);
		unsigned int RECONSTRUCTEDMESH_ID = GFXContentManager->Upload_MeshBuffer(TuranEditor::RenderDataManager::PositionNormal_VertexAttrib, VertexBuffer.data(), VertexBuffer.size() * 12, VertexPositions.size(), nullptr, 0);

		PCViewer::TriangleModel* newtrimodel = new PCViewer::TriangleModel;
		newtrimodel->NAME = IMPORTNAME + "_" + to_string(SDFResNaive * SDFResMultiplier);
		newtrimodel->PATH = "Generated";
		newtrimodel->isVisible = true;
		newtrimodel->GPUMESH_IDs.push_back(RECONSTRUCTEDMESH_ID);
		newtrimodel->DisplayedVertexBuffers.push_back(true);
		newtrimodel->NonIndexed_VertexBuffers.push_back(VertexPositions);
		vec3 BOUNDINGMIN = vec3(FLT_MAX), BOUNDINGMAX = vec3(FLT_MIN);
		for (unsigned int i = 0; i < VertexPositions.size(); i++) {
			BOUNDINGMIN.x = std::min(BOUNDINGMIN.x, VertexPositions[i].x);
			BOUNDINGMIN.y = std::min(BOUNDINGMIN.y, VertexPositions[i].y);
			BOUNDINGMIN.z = std::min(BOUNDINGMIN.z, VertexPositions[i].z);
			BOUNDINGMAX.x = std::max(BOUNDINGMAX.x, VertexPositions[i].x);
			BOUNDINGMAX.y = std::max(BOUNDINGMAX.y, VertexPositions[i].y);
			BOUNDINGMAX.z = std::max(BOUNDINGMAX.z, VertexPositions[i].z);
		}
		newtrimodel->BOUNDINGMAX = BOUNDINGMAX;
		newtrimodel->BOUNDINGMIN = BOUNDINGMIN;
		newtrimodel->CenterOfData = (BOUNDINGMAX - BOUNDINGMIN) / vec3(2.0);
		newtrimodel->BoundingSphereRadius = length(BOUNDINGMAX - BOUNDINGMIN) / 2.0f;
		PCViewer::TriangleModel::TriangleModelNormals trimodel_normaltype;
		trimodel_normaltype.NORMALNAME = "Generated";
		trimodel_normaltype.NonIndexed_VertexNormals.push_back(VertexNormals);
		newtrimodel->VertexNormalTYPEs.push_back(trimodel_normaltype);

		viewer->DisplayableDatas.push_back(newtrimodel);
	}
	if (samplepoint_renderer && ShowSDFVolume) {
		samplepoint_renderer->RenderThisFrame();
	}
}