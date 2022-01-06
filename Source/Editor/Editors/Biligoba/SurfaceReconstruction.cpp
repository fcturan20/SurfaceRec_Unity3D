#include "PointCloudTools.h"
#include "Editor/TUBITAK/Algorithms.h"
#include "TuranAPI/ThreadedJobCore.h"
#include "TuranAPI/Profiler_Core.h"

string IMPORTNAME = "";
PCViewer::TriangleModel* lastgeneratedmodel = nullptr;
unsigned int LASTUSED_selectedddindex = UINT32_MAX;
static PCViewer* Viewer = nullptr;

//Multi-threading
static bool is_multithreaded = false;
static std::atomic_uint processing_jobindex = 0, finished_jobindex = 0;
static unsigned int sdf_jobcount = 24, mc_jobcount = 24, sdf_elementcount_perjob, mc_elementcount_perjob, sdf_lastjob_elementcount, mc_lastjob_elementcount;
static vector<float> SDFValues;
static vector<vector<vec3>> MC_Results;
static unsigned int SDFRes = 0, selectedNormalListIndex = 0;
static float SamplingD = 0.0;
static dvec3 BOUNDINGMIN, BOUNDINGMAX, SAMPLE_DIST;
static PointCloud* CLOUD = nullptr;

inline vec3 FindSampleLoc_fromSampleIndex(unsigned int SamplePointIndex){
		unsigned int SampleIndex_Z = SamplePointIndex / (SDFRes * SDFRes);
		unsigned int SampleIndex_Y = (SamplePointIndex - (SampleIndex_Z * SDFRes * SDFRes)) / SDFRes;
		unsigned int SampleIndex_X = SamplePointIndex - (SampleIndex_Z * SDFRes * SDFRes) - (SampleIndex_Y * SDFRes);

		return vec3(
				BOUNDINGMIN.x + (SampleIndex_X * SAMPLE_DIST.x),
				BOUNDINGMIN.y + (SampleIndex_Y * SAMPLE_DIST.y),
				BOUNDINGMIN.z + (SampleIndex_Z * SAMPLE_DIST.z));
}

void MultiThreaded_SDFSampling(){
	unsigned int jobindex = processing_jobindex.fetch_add(1);
	const float maxsampledist = length(SAMPLE_DIST) * SamplingD / 2;
	const bool is_lastjob = (jobindex == sdf_jobcount - 1) ? true : false;
	const unsigned int thread_jobcount = (is_lastjob) ? (sdf_lastjob_elementcount) : (sdf_elementcount_perjob);
	for (unsigned int i = 0; i < thread_jobcount; i++) {
		unsigned int SamplePointIndex = (jobindex * sdf_elementcount_perjob) + i;
		vec3 SamplePoint = FindSampleLoc_fromSampleIndex(SamplePointIndex);

		unsigned int ClosestPCPointIndex = TuranEditor::Algorithms::Searchfor_ClosestNeighbors(*CLOUD, SamplePoint, 1)[0];
		vec3 Sample_toPCPoint = SamplePoint - CLOUD->PointPositions[ClosestPCPointIndex];
		float dist_fromsurface = length(Sample_toPCPoint) * dot(normalize(Sample_toPCPoint), CLOUD->PointNormals[selectedNormalListIndex].Normals[ClosestPCPointIndex]);

		if (length(Sample_toPCPoint) > maxsampledist) {
			if (dist_fromsurface < 0.0) {
				SDFValues[SamplePointIndex] = -FLT_MAX;
			}
		}
		else {
			SDFValues[SamplePointIndex] = dist_fromsurface;
		}
	}

	finished_jobindex.fetch_add(1);
}

#include "Editor/TUBITAK/MarchingCubes_LookUpTable.h"
void MultiThreaded_MC(){
	unsigned int jobindex = processing_jobindex.fetch_add(1);
	const float maxsampledist = length(SAMPLE_DIST) * SamplingD / 2;
	const bool is_lastjob = (jobindex == mc_jobcount - 1) ? true : false;
	const unsigned int thread_jobcount = (is_lastjob) ? mc_lastjob_elementcount : mc_elementcount_perjob;
	for (unsigned int i = 0; i < thread_jobcount; i++) {
		unsigned int CellIndex = (mc_elementcount_perjob * jobindex) + i;
		static constexpr uvec2 EdgeList[12] = {
			uvec2(0, 1), uvec2(1, 2), uvec2(2,3), uvec2(3,0),
			uvec2(4, 5), uvec2(5,6), uvec2(6,7), uvec2(7,4),
			uvec2(0, 4), uvec2(1,5), uvec2(2,6), uvec2(3,7)
		};
		unsigned int CellIndex_Z = CellIndex / ((SDFRes - 1) * (SDFRes - 1));
		unsigned int CellIndex_Y = (CellIndex - (CellIndex_Z * (SDFRes - 1) * (SDFRes - 1))) / (SDFRes - 1);
		unsigned int CellIndex_X = CellIndex - (CellIndex_Z * (SDFRes - 1) * (SDFRes - 1)) - (CellIndex_Y * (SDFRes - 1));

		unsigned int SampleIndexes[8] = { (CellIndex_Z * SDFRes * SDFRes) + (CellIndex_Y * SDFRes) + CellIndex_X,
		(CellIndex_Z * SDFRes * SDFRes) + (CellIndex_Y * SDFRes) + CellIndex_X + 1,
		((CellIndex_Z + 1) * SDFRes * SDFRes) + (CellIndex_Y * SDFRes) + CellIndex_X + 1,
		((CellIndex_Z + 1) * SDFRes * SDFRes) + (CellIndex_Y * SDFRes) + CellIndex_X,
		(CellIndex_Z * SDFRes * SDFRes) + ((CellIndex_Y + 1) * SDFRes) + CellIndex_X,
		(CellIndex_Z * SDFRes * SDFRes) + ((CellIndex_Y + 1) * SDFRes) + CellIndex_X + 1,
		((CellIndex_Z + 1) * SDFRes * SDFRes) + ((CellIndex_Y + 1) * SDFRes) + CellIndex_X + 1,
		((CellIndex_Z + 1) * SDFRes * SDFRes) + ((CellIndex_Y + 1) * SDFRes) + CellIndex_X
		};
		 
		unsigned char LookUpTable_Index = 0;
		for (unsigned char CornerIndex = 0; CornerIndex < 8; CornerIndex++) {
			if (abs(SDFValues[SampleIndexes[CornerIndex]]) != FLT_MAX &&
				SDFValues[SampleIndexes[CornerIndex]] < 0.0) { 
				LookUpTable_Index |= 1 << CornerIndex;
			}
		} 

		unsigned char Table_EdgeSearchIndex = 0;  
		while (MarchingCubes_LookUpTable[LookUpTable_Index][Table_EdgeSearchIndex] != -1) {
			unsigned char EdgeIndex = MarchingCubes_LookUpTable[LookUpTable_Index][Table_EdgeSearchIndex];
			unsigned int Sample0 = SampleIndexes[EdgeList[EdgeIndex].x], Sample1 = SampleIndexes[EdgeList[EdgeIndex].y];

			//If one of the samples is infinite, place the vertex using only non-infinite sample's distance
			if (abs(SDFValues[Sample0]) == FLT_MAX || abs(SDFValues[Sample1]) == FLT_MAX) {
				float dist = abs(SDFValues[Sample0]) == FLT_MAX ? SDFValues[Sample1] : SDFValues[Sample0];
				unsigned int ActiveSample = SDFValues[Sample0] == dist ? Sample0 : Sample1, InfiniteSample = SDFValues[Sample0] == dist ? Sample1 : Sample0;
				vec3 ActiveSamplePos = FindSampleLoc_fromSampleIndex(ActiveSample), InfiniteSamplePos = FindSampleLoc_fromSampleIndex(InfiniteSample);

				MC_Results[jobindex].push_back(ActiveSamplePos + (normalize(ActiveSamplePos - InfiniteSamplePos) * std::min(dist, length(ActiveSamplePos - InfiniteSamplePos))));
			}
			else {
				unsigned int InsideSample = SDFValues[Sample0] < SDFValues[Sample1] ? Sample0 : Sample1, OutsideSample = SDFValues[Sample0] < SDFValues[Sample1] ? Sample1 : Sample0;
				vec3 InsideSamplePos = FindSampleLoc_fromSampleIndex(InsideSample), OutsideSamplePos = FindSampleLoc_fromSampleIndex(OutsideSample);
				vec3 Inside_to_Outside = OutsideSamplePos - InsideSamplePos;

				vec3 finalpos = InsideSamplePos + (normalize(Inside_to_Outside) * (abs(SDFValues[InsideSample]) * length(Inside_to_Outside) / (abs(SDFValues[Sample0]) + abs(SDFValues[Sample1]))));
				MC_Results[jobindex].push_back(finalpos);
			}
			 
			Table_EdgeSearchIndex++;
		}
	}

	unsigned int VertexCount = MC_Results[jobindex].size();
	MC_Results[jobindex].resize(MC_Results[jobindex].size() * 2, vec3(0.0));
	for (unsigned int FaceID = 0; FaceID < MC_Results[jobindex].size() / 6; FaceID++) {
		vec3 v[3] = { MC_Results[jobindex][VertexCount + (FaceID * 3)] , MC_Results[jobindex][VertexCount + (FaceID * 3) + 1] , MC_Results[jobindex][VertexCount + (FaceID * 3) + 2] };

		if (!CLOUD->PointNormals.size()) { LOG_CRASHING("Invalid input, reference point cloud should have normals!");}
		for (unsigned int GeneratedVertexIndex = 0; GeneratedVertexIndex < 3; GeneratedVertexIndex++) {
			vec3 RefNormal = CLOUD->PointNormals[0].Normals[TuranEditor::Algorithms::Searchfor_ClosestNeighbors(*CLOUD, v[GeneratedVertexIndex], 1)[0]];
			vec3 normal = cross(v[(GeneratedVertexIndex + 1) % 3] - v[GeneratedVertexIndex], v[(GeneratedVertexIndex + 2) % 3] - v[GeneratedVertexIndex]);
			normal *= sign(dot(RefNormal, normal));
			MC_Results[jobindex][VertexCount + (FaceID * 3) + GeneratedVertexIndex] = normalize(normal);
		}
	}
	finished_jobindex.fetch_add(1);
}


void SurfaceRec() {
	IMGUI->Input_Text("Reconstructed Model Name", &IMPORTNAME); 

	static unsigned int selectedlistitemindex = 0, selectedddindex = 0;
	Viewer->SelectListOneLine_FromDisplayableDatas(PCViewer::DisplayableData::POINTCLOUD, selectedlistitemindex, selectedddindex, "Point Cloud");
	if (selectedddindex != UINT32_MAX) {
		PCViewer::PointCloud_DD* pc_dd = static_cast<PCViewer::PointCloud_DD*>(Viewer->DisplayableDatas[selectedddindex]);

		static int SDFResNaive = 10, SDFResMultiplier = 1, kNNCount = 2;
		static bool ShowSDFVolume = false;

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
		IMGUI->Slider_Float("SamplingD", &SamplingD, 1.0 / float(SDFRes), SDFResNaive);
		SDFRes = SDFResNaive * SDFResMultiplier;
		IMGUI->Checkbox("Show SDF Volume", &ShowSDFVolume);
		IMGUI->Same_Line();
		IMGUI->Checkbox("Multi-Threaded", &is_multithreaded);
		if (IMGUI->Button("Reconstruct") && selectedNormalListIndex != UINT32_MAX) {
			TuranEditor::Algorithms::Generate_KDTree(pc_dd->PC);

			if(is_multithreaded){
				SDFValues.clear();
				SDFValues.resize(SDFRes * SDFRes * SDFRes, FLT_MAX);
				MC_Results.resize(mc_jobcount);
				CLOUD = &pc_dd->PC;
				
				dvec3 CUBESIZE = pc_dd->BOUNDINGMAX - pc_dd->BOUNDINGMIN;
				BOUNDINGMAX = pc_dd->BOUNDINGMAX  + (CUBESIZE / 20.0);
				BOUNDINGMIN = pc_dd->BOUNDINGMIN - (CUBESIZE / 20.0);
				CUBESIZE *= 1.1;
				SAMPLE_DIST = CUBESIZE / dvec3(SDFRes - uvec3(1));
				sdf_elementcount_perjob = (SDFRes * SDFRes * SDFRes) / sdf_jobcount;
				sdf_lastjob_elementcount = sdf_elementcount_perjob + ((SDFRes * SDFRes * SDFRes) % sdf_jobcount);
				mc_elementcount_perjob = ((SDFRes - 1) * (SDFRes - 1) * (SDFRes - 1)) / mc_jobcount;
				mc_lastjob_elementcount = mc_elementcount_perjob + (((SDFRes - 1) * (SDFRes - 1) * (SDFRes - 1)) % mc_jobcount);
				
				{
					TURAN_PROFILE_SCOPE("Reconstruction");
					for(unsigned int i = 0; i < sdf_jobcount; i++){
						tapi_Execute_withoutWait(threading_system, MultiThreaded_SDFSampling);
					}
					tapi_waitForAllOtherJobs(threading_system);
					if(finished_jobindex.load() != sdf_jobcount){
						LOG_CRASHING("SDF Sync failed!");
					}
					processing_jobindex.store(0); finished_jobindex.store(0);
					for(unsigned int i = 0; i < mc_jobcount; i++){
						tapi_Execute_withoutWait(threading_system, MultiThreaded_MC);
					}
					tapi_waitForAllOtherJobs(threading_system);
					if(finished_jobindex.load() != mc_jobcount){
						LOG_CRASHING("MC Sync failed!");
					}
					processing_jobindex.store(0); finished_jobindex.store(0);
				}
				LOG_CRASHING("Reconstruction finished!");
				

				unsigned int vertexbuffersize = 0;
				for(unsigned int i = 0; i < MC_Results.size(); i++){
					vertexbuffersize += MC_Results[i].size();
				}
				vector<vec3> Final_VertexBuffer(vertexbuffersize);

				unsigned int last_memcpy_location = 0;
				for(unsigned int i = 0; i < MC_Results.size(); i++){
					memcpy(Final_VertexBuffer.data() + last_memcpy_location, MC_Results[i].data(), MC_Results[i].size() * 6);
					memcpy(Final_VertexBuffer.data() + (vertexbuffersize / 2) + last_memcpy_location, MC_Results[i].data() + (MC_Results[i].size() / 2), MC_Results[i].size() * 6);
					last_memcpy_location += MC_Results[i].size() / 2;
				}

				unsigned int RECONSTRUCTEDMESH_ID = GFXContentManager->Upload_MeshBuffer(TuranEditor::RenderDataManager::PositionNormal_VertexAttrib, Final_VertexBuffer.data(), Final_VertexBuffer.size() * 12, Final_VertexBuffer.size() / 2, nullptr, 0);

				PCViewer::TriangleModel* trimodel = new PCViewer::TriangleModel;
				trimodel->NAME = IMPORTNAME + to_string(SDFRes) + "x" + to_string(SDFRes) + "-e" + to_string(SamplingD);
				trimodel->PATH = "Generated";
				trimodel->isVisible = true;
				trimodel->GPUMESH_IDs.push_back(RECONSTRUCTEDMESH_ID);
				trimodel->DisplayedVertexBuffers.push_back(true);
				PCViewer::TriangleModel::TriangleModelNormals normallistobj;
				normallistobj.NORMALNAME = "MC_Normals";
				trimodel->VertexNormalTYPEs.push_back(normallistobj);
				trimodel->BOUNDINGMAX = BOUNDINGMAX;
				trimodel->BOUNDINGMIN = BOUNDINGMIN;
				trimodel->BoundingSphereRadius = length(trimodel->BOUNDINGMAX - trimodel->BOUNDINGMIN) / 2.0;
				trimodel->CenterOfData = trimodel->BOUNDINGMIN + ((trimodel->BOUNDINGMAX - trimodel->BOUNDINGMIN) / dvec3(2.0));
				lastgeneratedmodel = trimodel;
				LASTUSED_selectedddindex = selectedddindex;
				
			
				Viewer->DisplayableDatas.push_back(trimodel);
			}
			else{
				
				unsigned int RECONSTRUCTEDMESH_ID = UINT32_MAX;
				vector<vec3> SampleLocations;
				vector<float> SDFValues = TuranEditor::Algorithms::SDF_FromPointCloud(&pc_dd->PC, uvec3(SDFRes), selectedNormalListIndex, SamplingD, &SampleLocations);
				samplepoint_renderer = TuranEditor::RenderDataManager::Create_PointRenderer(SDFValues.size());
				for (unsigned int i = 0; i < SDFValues.size(); i++) {
					samplepoint_renderer->GetPointPosition_byIndex(i) = SampleLocations[i];
					if (SDFValues[i] == FLT_MAX) { samplepoint_renderer->GetPointCOLORRGBA_byIndex(i) = vec4(0, 0, 1.0, 0.0); }
					else if (SDFValues[i] == -FLT_MAX) { samplepoint_renderer->GetPointCOLORRGBA_byIndex(i) = vec4(0, 1.0, 0.0, 0.0); }
					else { samplepoint_renderer->GetPointCOLORRGBA_byIndex(i) = vec4(vec3((SDFValues[i] + 1.0) / 2.0), 1.0); }
				}
				vector<vec3> VertexNormals;
				vector<vec3> VertexPositions = TuranEditor::Algorithms::MarchingCubes(uvec3(SDFRes), SDFValues, SampleLocations, &pc_dd->PC, &VertexNormals);
				vector<vec3> VertexBuffer(VertexPositions.size() * 2, vec3(0.0));
				memcpy(VertexBuffer.data(), VertexPositions.data(), VertexPositions.size() * 12);
				memcpy(VertexBuffer.data() + VertexPositions.size(), VertexNormals.data(), VertexNormals.size() * 12);

				RECONSTRUCTEDMESH_ID = GFXContentManager->Upload_MeshBuffer(TuranEditor::RenderDataManager::PositionNormal_VertexAttrib, VertexBuffer.data(), VertexBuffer.size() * 12, VertexPositions.size(), nullptr, 0);

				PCViewer::TriangleModel* trimodel = new PCViewer::TriangleModel;
				trimodel->NAME = IMPORTNAME + to_string(SDFRes) + "x" + to_string(SDFRes) + "-e" + to_string(SamplingD);
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

		}
		if (ShowSDFVolume && samplepoint_renderer) {
			samplepoint_renderer->RenderThisFrame();
		}
	}
}

void PointCloudTools::SurfaceReconstruction(PCViewer* viewer) {
	Viewer = viewer;

		SurfaceRec();
}