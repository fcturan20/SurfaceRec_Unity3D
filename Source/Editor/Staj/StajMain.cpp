#include "Editor/Editor.h"
using namespace TuranEditor;
#include "Editor/FileSystem/ResourceTypes/ResourceTYPEs.h"

int StajMain() {
	Editor_System* EDITORSYSTEM = new Editor_System;
	Game_RenderGraph First_RenderGraph;
	Main_Window* main_window = new Main_Window(&First_RenderGraph);



	const char* MODEL_PATH = "C:/Users/furka/Desktop/Meshes/cat0.off";
	unsigned int MODEL_ID = Model_Importer::Import_Model(MODEL_PATH);

	//Create Material Instances to render lines
	unsigned int BlueLine_MatInstID = 0, RedLine_MatInstID = 0;
	//Blue Line
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

	//Red Line
	GFX_API::Material_Instance* RedLine_MatInst = new GFX_API::Material_Instance;
	RedLine_MatInst->Material_Type = TuranEditor::Editor_RendererDataManager::PointLineMaterialID;

	//Uniform Preparing
	RedLine_MatInst->UNIFORM_LIST = POINTLINE_MATTYPE->UNIFORMs;
	for (unsigned int i = 0; i < RedLine_MatInst->UNIFORM_LIST.size(); i++) {
		GFX_API::Material_Uniform& UNIFORM = RedLine_MatInst->UNIFORM_LIST[i];
		if (UNIFORM.VARIABLE_NAME == "POINTCOLOR") {
			UNIFORM.DATA = new vec3(1, 0, 0);
		}
		else if (UNIFORM.VARIABLE_NAME == "POINT_SIZE") {
			UNIFORM.DATA = new float(5);
		}
		else if (UNIFORM.VARIABLE_NAME == "OBJECT_INDEX") {
			UNIFORM.DATA = new unsigned int(0);
		}
	}
	Resource_Identifier* redRESOURCE = new Resource_Identifier;
	redRESOURCE->TYPE = TuranEditor::RESOURCETYPEs::GFXAPI_SHADERSOURCE;
	redRESOURCE->DATA = RedLine_MatInst;
	EDITOR_FILESYSTEM->Add_anAsset_toAssetList(redRESOURCE);
	RedLine_MatInstID = redRESOURCE->ID;


	if (!MODEL_ID) {
		LOG_WARNING("There is not any object to display!\n");
	}

	Static_Model* MODEL = (Static_Model*)EDITOR_FILESYSTEM->Find_ResourceIdentifier_byID(MODEL_ID)->DATA;


	unsigned int SourceVertexIndex = 0, TargetVertexIndex = 150;
	GFX_API::VertexAttributeLayout PositionOnlyVertexAttribLayout;
	PositionOnlyVertexAttribLayout.Attributes.push_back(GFX_API::VertexAttribute());
	GFX_API::VertexAttribute& Attrib = PositionOnlyVertexAttribLayout.Attributes[0];
	Attrib.AttributeName = "Positions";
	Attrib.DATATYPE = GFX_API::DATA_TYPE::VAR_VEC3;
	Attrib.Index = 0;
	Attrib.Start_Offset = 0;
	Attrib.Stride = 3;
	PositionOnlyVertexAttribLayout.Calculate_SizeperVertex();

	//FPSed Geodesic Path Searching vs Exact Dijkstra Geodesics
	{
		//FPSed Geodesic Path Search
		PreprocessedMesh CatLowFPSed;
		bool PreprocessFirst = false;
		const char* ModelPath = "C:/Users/furka/Desktop/CompiledData.trn";
		if (PreprocessFirst) {
			CatLowFPSed.PreProcessMesh(*MODEL->MESHes[0], 10);
			CatLowFPSed.StorePreProcessedData(ModelPath);
		}
		else {
			CatLowFPSed.LoadFromDisk(ModelPath);
		}

		//Find Dijkstra Distances
		vector<DijkstraVertex> DijkstraVertexes;
		Find_AllShortestPaths(*MODEL->MESHes[0], 150, DijkstraVertexes);


		//Get Visualization Datas
		VisualizationBuffer FPSVisBuf, DijkstraVisBuf;
		CatLowFPSed.GetVisualizationBufferData(150, FPSVisBuf);
		GetVisData_FromDijkstraVertexes(DijkstraVertexes, DijkstraVisBuf);


		float MAXDIF = FLT_MIN;
		vector<float> FPSDistances, DijkstraDistances;
		FPSVisBuf.GetDistances(FPSDistances);
		DijkstraVisBuf.GetDistances(DijkstraDistances);
		vector<float> FinalColorizationData;
		FinalColorizationData.resize(FPSDistances.size());
		for (unsigned int VertexID = 0; VertexID < MODEL->MESHes[0]->VERTEX_NUMBER; VertexID++) {
			FinalColorizationData[VertexID] = abs(FPSDistances[VertexID] - DijkstraDistances[VertexID]);
			if (FinalColorizationData[VertexID] > MAXDIF) {
				MAXDIF = FinalColorizationData[VertexID];
			}
		}
		//LOG_CRASHING("Max Difference: " + to_string(MAXDIF));

		VisualizationBuffer FinalVisBuf;
		FinalVisBuf.SetDatas(FinalColorizationData, MAXDIF);


		unsigned data_size = 0;
		Editor_RendererDataManager::UpdateGeodesicDistances(FinalVisBuf.GetGPUData(data_size), data_size);
	}


	//Flip Geodesics Path Algorithm
	{
		/*
		unsigned int POINTBUFFERID = 0;
		vector<vec3> Path = Find_FlipGeodesics(MODEL_PATH, SourceVertexIndex, TargetVertexIndex);
		POINTBUFFERID = GFXContentManager->Upload_PointBuffer(PositionOnlyVertexAttribLayout, Path.data(), Path.size());
		if (!POINTBUFFERID) {
			LOG_CRASHING("Uploading the SPF to the GPU has failed!");
		}
		else {
		}

		GFX_API::PointLineDrawCall FlipG_DrawCall;
		FlipG_DrawCall.Draw_asPoint = false;
		FlipG_DrawCall.PointBuffer_ID = POINTBUFFERID;
		FlipG_DrawCall.ShaderInstance_ID = RedLine_MatInstID;
		First_RenderGraph.Register_PointDrawCall(FlipG_DrawCall);
		*/
	}

	/*
	//Dijkstra Loop Path Algorithm
	{
		unsigned int POINTBUFFERID = 0;
		vector<unsigned int> VertexList{ 51, 86, 79 };
		vector<vec3> Path = Find_DijkstraPathLoop(MODEL_PATH, VertexList, true);
		POINTBUFFERID = GFXContentManager->Upload_PointBuffer(PositionOnlyVertexAttribLayout, Path.data(), Path.size());
		if (!POINTBUFFERID) {
			LOG_CRASHING("Uploading the SPF to the GPU has failed!");
		}

		GFX_API::PointLineDrawCall FlipG_DrawCall;
		FlipG_DrawCall.Draw_asPoint = false;
		FlipG_DrawCall.PointBuffer_ID = POINTBUFFERID;
		FlipG_DrawCall.ShaderInstance_ID = RedLine_MatInstID;
		First_RenderGraph.Register_PointDrawCall(FlipG_DrawCall);
	}*/


	for (unsigned int i = 0; i < MODEL->MESHes.size(); i++) {
		Static_Mesh_Data* MESH = MODEL->MESHes[i];
		unsigned int MESHBUFFER_ID = GFXContentManager->Upload_MeshBuffer(MESH->DataLayout, MESH->VERTEX_DATA, MESH->VERTEXDATA_SIZE, MESH->VERTEX_NUMBER, MESH->INDEX_DATA, MESH->INDICES_NUMBER);

		GFX_API::DrawCall Mesh_DrawCall;
		Mesh_DrawCall.MeshBuffer_ID = MESHBUFFER_ID;
		Mesh_DrawCall.ShaderInstance_ID = MODEL->MATINST_IDs[MESH->Material_Index];
		Mesh_DrawCall.JoinedDrawPasses = 0xffffffff;	//Join all draw passes for now!

		First_RenderGraph.Register_DrawCall(Mesh_DrawCall);
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