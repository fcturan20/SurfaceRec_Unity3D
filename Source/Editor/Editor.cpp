#include "Editor.h"
using namespace TuranEditor;

int main() {
	Editor_System* EDITORSYSTEM = new Editor_System;
	Game_RenderGraph First_RenderGraph;
	Main_Window* main_window = new Main_Window(&First_RenderGraph);


	//Load Model
	const char* CAT = "C:/Users/furka/Desktop/Meshes/cat0.off";
	const char* HUMAN = "C:/Users/furka/Desktop/Meshes/SCAPE/Meshes/mesh000.off";
	const char* CLOUDPATH = HUMAN;
	PointCloud* CLOUD = DataLoader::LoadMesh_asPointCloud(CLOUDPATH);
	if (!CLOUD) {
		LOG_CRASHING("Point Cloud import has failed!");
	}

	//Create Vertex Attribute
	GFX_API::VertexAttributeLayout PositionNormal_VertexAttrib, PositionOnly_VertexAttrib;
	{
		PositionNormal_VertexAttrib.Attributes.push_back(GFX_API::VertexAttribute());
		GFX_API::VertexAttribute& PosAttrib = PositionNormal_VertexAttrib.Attributes[0];
		PosAttrib.AttributeName = "Positions";
		PosAttrib.DATATYPE = GFX_API::DATA_TYPE::VAR_VEC3;
		PosAttrib.Index = 0;
		PosAttrib.Start_Offset = 0;
		PosAttrib.Stride = 3;

		PositionNormal_VertexAttrib.Attributes.push_back(GFX_API::VertexAttribute());
		GFX_API::VertexAttribute& NorAttrib = PositionNormal_VertexAttrib.Attributes[1];
		NorAttrib.AttributeName = "Normals";
		NorAttrib.DATATYPE = GFX_API::DATA_TYPE::VAR_VEC3;
		NorAttrib.Index = 1;
		NorAttrib.Start_Offset = 0;
		NorAttrib.Stride = 3;

		PositionOnly_VertexAttrib.Attributes.push_back(GFX_API::VertexAttribute());
		GFX_API::VertexAttribute& PosAttrib2 = PositionOnly_VertexAttrib.Attributes[0];
		PosAttrib2.AttributeName = "Positions";
		PosAttrib2.DATATYPE = GFX_API::DATA_TYPE::VAR_VEC3;
		PosAttrib2.Index = 0;
		PosAttrib2.Start_Offset = 0;
		PosAttrib2.Stride = 3;
	}
	PositionNormal_VertexAttrib.Calculate_SizeperVertex();
	PositionOnly_VertexAttrib.Calculate_SizeperVertex();
	

	//Create Material Instance for Point Cloud
	unsigned int BlueLine_MatInstID = 0;
	{
		GFX_API::Material_Type* POINTLINE_MATTYPE = (GFX_API::Material_Type*)TuranEditor::EDITOR_FILESYSTEM->Find_ResourceIdentifier_byID(TuranEditor::Editor_RendererDataManager::PointLineMaterialID)->DATA;
		GFX_API::Material_Instance* BlueLine_MatInst = new GFX_API::Material_Instance;
		BlueLine_MatInst->Material_Type = TuranEditor::Editor_RendererDataManager::PointLineMaterialID;

		//Uniform Preparing
		BlueLine_MatInst->UNIFORM_LIST = POINTLINE_MATTYPE->UNIFORMs;
		for (unsigned int i = 0; i < BlueLine_MatInst->UNIFORM_LIST.size(); i++) {
			GFX_API::Material_Uniform& UNIFORM = BlueLine_MatInst->UNIFORM_LIST[i];
			if (UNIFORM.VARIABLE_NAME == "POINTCOLOR") {
				UNIFORM.DATA = new vec3(0, 0, 1);
			}
			else if (UNIFORM.VARIABLE_NAME == "POINT_SIZE") {
				UNIFORM.DATA = new float(5);
			}
			else if (UNIFORM.VARIABLE_NAME == "OBJECT_INDEX") {
				UNIFORM.DATA = new unsigned int(0);
			}
		}

		Resource_Identifier* RESOURCE = new Resource_Identifier;
		RESOURCE->TYPE = TuranEditor::RESOURCETYPEs::GFXAPI_SHADERSOURCE;
		RESOURCE->DATA = BlueLine_MatInst;
		EDITOR_FILESYSTEM->Add_anAsset_toAssetList(RESOURCE);
		BlueLine_MatInstID = RESOURCE->ID;
	}

	//Draw Call
	//unsigned int PointBufferID = GFXContentManager->Upload_PointBuffer(PositionNormal_VertexAttrib, CLOUD->PointPositions, CLOUD->PointCount);
	GFX_API::PointLineDrawCall PointCloud_DrawCall;
	PointCloud_DrawCall.Draw_asPoint = true;
	//PointCloud_DrawCall.PointBuffer_ID = PointBufferID;
	PointCloud_DrawCall.ShaderInstance_ID = BlueLine_MatInstID;
	//First_RenderGraph.Register_PointDrawCall(PointCloud_DrawCall);
	

	{

		vector<vec3> TriangulatedCloudPositions(CLOUD->PointCount * 400);
		vector<vec3> TriangulatedCloudNormals(TriangulatedCloudPositions.size());
		unsigned int LastUsedIndex = 0;
		Algorithms::Generate_KDTree(*CLOUD);
		for (unsigned int PointIndex = 0; PointIndex < CLOUD->PointCount; PointIndex++) {
			vector<vec3> PointList = Algorithms::Searchfor_ClosestNeighbors(*CLOUD, CLOUD->PointPositions[PointIndex], 50, true);

			vector<vec3> PCA = Algorithms::Compute_PCA(PointList);
			vector<vec3> ProjectedPoints = Algorithms::ProjectPoints_OnPlane_thenTriangulate(PointList, PCA[0], PCA[1], PCA[2]);
			

			for (unsigned int TriangulatedPointIndex = 0; TriangulatedPointIndex < ProjectedPoints.size(); TriangulatedPointIndex++) {
				TriangulatedCloudPositions[LastUsedIndex] = ProjectedPoints[TriangulatedPointIndex];
				TriangulatedCloudNormals[LastUsedIndex] = PCA[2];
				LastUsedIndex++;
			}
		}
		{
			vector<vec3> TriangulatedFinalDatas(TriangulatedCloudPositions.size() * 2);
			for (unsigned int VertexID = 0; VertexID < TriangulatedCloudPositions.size(); VertexID++) {
				TriangulatedFinalDatas[VertexID] = TriangulatedCloudPositions[VertexID];
				TriangulatedFinalDatas[TriangulatedCloudPositions.size() + VertexID] = TriangulatedCloudNormals[VertexID];
			}
			unsigned int MESHBUFFER_ID = GFXContentManager->Upload_MeshBuffer(PositionNormal_VertexAttrib, TriangulatedFinalDatas.data(),
				TriangulatedFinalDatas.size() * 12, TriangulatedCloudPositions.size(), nullptr, 0);
			GFX_API::DrawCall Mesh_DrawCall;
			Mesh_DrawCall.MeshBuffer_ID = MESHBUFFER_ID;
			unsigned int MeshNumber = 0;
			Mesh_DrawCall.ShaderInstance_ID = Editor_RendererDataManager::Create_SurfaceMaterialInstance(TuranEditor::SURFACEMAT_PROPERTIES(), &MeshNumber);
			Mesh_DrawCall.JoinedDrawPasses = 0xffffffff;	//Join all draw passes for now!

			First_RenderGraph.Register_DrawCall(Mesh_DrawCall);
		}
	}

	long long FrameTime = 0;
	while (main_window->Get_Is_Window_Open()) {
		FrameTime /= 1000;	//Convert from macroseconds to milliseconds
		Editor_System::Set_DeltaTime(FrameTime);
		LOG_STATUS("Delta time in float: " + to_string(float(FrameTime)));
		TURAN_PROFILE_SCOPE_O("Run Loop", &FrameTime);

		Editor_RendererDataManager::Update_GPUResources();
		IMGUI->New_Frame();
		IMGUI_RUNWINDOWS();
		GFX->Swapbuffers_ofMainWindow();
		IMGUI->Render_Frame();
		GFX->Run_RenderGraphs();

		//Take inputs by GFX API specific library that supports input (For now, just GLFW for OpenGL4) and send it to Engine!
		//In final product, directly OS should be used to take inputs!
		Editor_System::Take_Inputs();
	}


	return 1;
}