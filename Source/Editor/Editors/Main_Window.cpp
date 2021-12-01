#include "Main_Window.h"

#include "GFX/GFX_Core.h"
#include "Editor/RenderContext/Game_RenderGraph.h"
#include "GFX/GFX_Core.h"
#include "Editor/RenderContext/Editor_DataManager.h"
#include "Editor/Editor.h"
#include <string>

using namespace TuranAPI;

bool ShowNormal = true;

Main_Window::Main_Window(GFX_API::VertexAttributeLayout& PositionNormal_VertexAttrib, Game_RenderGraph* game_rendergraph) : IMGUI_WINDOW("Main_Window"),
GameRenderGraph(game_rendergraph), VAL(PositionNormal_VertexAttrib),
SurfaceMatInst(TuranEditor::RenderDataManager::Create_SurfaceMaterialInstance(TuranEditor::SURFACEMAT_PROPERTIES(), new unsigned int(0))) {
	IMGUI_REGISTERWINDOW(this);

	//Import original models with indexes
	const char* CAT = "C:/Users/furka/Desktop/Meshes/CatBase.off";
	const char* HUMAN = "C:/Users/furka/Desktop/Meshes/HumanBase.off";
	unsigned int CAT_ASSETID = TuranEditor::Model_Importer::Import_Model(CAT);
	unsigned int HUMAN_ASSETID = TuranEditor::Model_Importer::Import_Model(HUMAN);

	if (//!CAT_ASSETID || 
		!HUMAN_ASSETID) {
		LOG_WARNING("Failed to import Original objects!\n");
	}

	//Send original models with indexes to GPU 
	//Static_Model* CAT_MODEL = (Static_Model*)EDITOR_FILESYSTEM->Find_ResourceIdentifier_byID(CAT_ASSETID)->DATA;
	TuranEditor::Static_Model* HUMAN_MODEL = (TuranEditor::Static_Model*)TuranEditor::EDITOR_FILESYSTEM->Find_ResourceIdentifier_byID(HUMAN_ASSETID)->DATA;
	/*for (unsigned int i = 0; i < CAT_MODEL->MESHes.size(); i++) {
		Static_Mesh_Data* MESH = CAT_MODEL->MESHes[i];
		CatOriginal.push_back(GFXContentManager->Upload_MeshBuffer(MESH->DataLayout, MESH->VERTEX_DATA, MESH->VERTEXDATA_SIZE, MESH->VERTEX_NUMBER,
			MESH->INDEX_DATA, MESH->INDICES_NUMBER));
	}*/
	for (unsigned int i = 0; i < HUMAN_MODEL->MESHes.size(); i++) {
		TuranEditor::Static_Mesh_Data* MESH = HUMAN_MODEL->MESHes[i];
		HumanOriginal.push_back(GFXContentManager->Upload_MeshBuffer(MESH->DataLayout, MESH->VERTEX_DATA, MESH->VERTEXDATA_SIZE, MESH->VERTEX_NUMBER,
			MESH->INDEX_DATA, MESH->INDICES_NUMBER));
	}




	//Import same models as point clouds and reconstruct their surfaces
	//PointCloud* CAT_PC = DataLoader::LoadMesh_asPointCloud(CAT);
	PointCloud* HUMAN_PC = TuranEditor::DataLoader::LoadMesh_asPointCloud(HUMAN);
	//Algorithms::Generate_KDTree(*HUMAN_PC);
	//HumanReconstructed.push_back(Algorithms::ReconstructSurface(HUMAN_PC, 0, PositionNormal_VertexAttrib, 3));

	//RECONSTRUCTION
	/*
	CatPCs.resize(ReconstructionTypeCount, *CAT_PC);
	HumanPCs.resize(ReconstructionTypeCount, *HUMAN_PC);

	CatK.resize(ReconstructionTypeCount, 9); HumanK.resize(ReconstructionTypeCount, 9);
	CatReconstructed.resize(ReconstructionTypeCount, UINT32_MAX); HumanReconstructed.resize(ReconstructionTypeCount, UINT32_MAX);
	const char* HumanSavePath = "C:/Users/furka/Desktop/ReconstructedSurface.rec";

	for (unsigned int ReconstructionTypeIndex = 0; ReconstructionTypeIndex < ReconstructionTypeCount; ReconstructionTypeIndex++) {
		HumanReconstructed[ReconstructionTypeIndex] = Algorithms::ReconstructSurface(&HumanPCs[ReconstructionTypeIndex], HumanK[ReconstructionTypeIndex],
			PositionNormal_VertexAttrib, ReconstructionTypeIndex);
		CatReconstructed[ReconstructionTypeIndex] = Algorithms::ReconstructSurface(&CatPCs[ReconstructionTypeIndex], CatK[ReconstructionTypeIndex], PositionNormal_VertexAttrib, ReconstructionTypeIndex);
	}*/

	//LOAD PREVIOUS RECONSTRUCTION FROM DISK
	/*
	unsigned char LoadedKNN, LoadedRecType;
	unsigned int MeshBuffer = Algorithms::LoadReconstructedSurface_fromDisk(HumanSavePath, PositionNormal_VertexAttrib, LoadedKNN, LoadedRecType);
	if (MeshBuffer == UINT32_MAX) {
		LOG_CRASHING("Loading of the reconstructed surface has failed!");
	}
	HumanReconstructed[LoadedRecType] = MeshBuffer;*/

	//NORMAL ESTIMATION
	//Upload original point cloud with normals
	unsigned int PC_OriginalNormalsList_INDEX = UINT32_MAX;
	for (unsigned int i = 0; i < HUMAN_PC->PointNormals.size(); i++) {
		if (HUMAN_PC->PointNormals[i].NAME == PC_PointNormals::ORIGINAL_NORMALNAME) {
			PC_OriginalNormalsList_INDEX = i;
		}
	}
	if (PC_OriginalNormalsList_INDEX != UINT32_MAX) {
		//Send Point Mesh Buffers
		{
			//Original
			vector<vec3> OriginalHumanPC_Datas(HUMAN_PC->PointCount * 2);
			for (unsigned int VertexID = 0; VertexID < HUMAN_PC->PointCount; VertexID++) {
				OriginalHumanPC_Datas[VertexID] = HUMAN_PC->PointPositions[VertexID];
				OriginalHumanPC_Datas[HUMAN_PC->PointCount + VertexID] = HUMAN_PC->PointNormals[PC_OriginalNormalsList_INDEX].Normals[VertexID];
			}
			OriginalHuman_PC = GFXContentManager->Create_PointBuffer(PositionNormal_VertexAttrib, OriginalHumanPC_Datas.data(), HUMAN_PC->PointCount);


			//Upload Hough based Normal Reconstructed Point Cloud
			vector<vec3> RobustNormalHumanPC_Datas(HUMAN_PC->PointCount * 2);
			vector<vec3> NormalList = TuranEditor::Algorithms::HoughBasedNormalReconstruction(*HUMAN_PC, true, 100, 1000, 15, 5, false, 0.79f, 4);
			for (unsigned int VertexID = 0; VertexID < HUMAN_PC->PointCount; VertexID++) {
				RobustNormalHumanPC_Datas[VertexID] = HUMAN_PC->PointPositions[VertexID];
				RobustNormalHumanPC_Datas[HUMAN_PC->PointCount + VertexID] = NormalList[VertexID];
			}
			RobustNormalHuman_PC = GFXContentManager->Create_PointBuffer(PositionNormal_VertexAttrib, RobustNormalHumanPC_Datas.data(), HUMAN_PC->PointCount);
		}

	}
	else {
		LOG_CRASHING("Point cloud doesn't have normals by default!");
	}
}

//Create main menubar for the Editor's main window!
void Main_Menubar_of_Editor();

/*
//This is for mesh reconstruction, normal construction is down below!
void Main_Window::Run_Window() {
	TURAN_PROFILE_SCOPE("Main Window!");
	if (!Is_Window_Open) {
		IMGUI->Is_IMGUI_Open = false;
		IMGUI_DELETEWINDOW(this);
		return;
	}
	if (!IMGUI->Create_Window("Main Window", Is_Window_Open, true)) {
		IMGUI->End_Window();
		return;
	}
	//Successfully created the window!
	Main_Menubar_of_Editor();


	IMGUI->End_Window();
}*/



void Main_Window::Run_Window() {
	TURAN_PROFILE_SCOPE("Main Window!");
	if (!Is_Window_Open) {
		IMGUI_DELETEWINDOW(this);
		return;
	}
	if (!IMGUI->Create_Window("Main Window", Is_Window_Open, true)) {
		IMGUI->End_Window();
		return;
	}
	//Successfully created the window!
	Main_Menubar_of_Editor();

	if (IMGUI->Begin_TabBar()) {
		if (IMGUI->Begin_TabItem("Lights")) {
			IMGUI->Slider_Vec3("Directional Light Color", (vec3*)&TuranEditor::RenderDataManager::DIRECTIONALLIGHTs[0].COLOR, 0, 1);
			IMGUI->Slider_Vec3("Directional Light Direction", (vec3*)&TuranEditor::RenderDataManager::DIRECTIONALLIGHTs[0].DIRECTION, -1, 1);
			IMGUI->End_TabItem();
		}
		ivec2 ObjPositionLimits;
		IMGUI->Checkbox("Show Normals Merged", &ShowOriginalRobustMerged);
		if (ShowOriginalRobustMerged) {
			if (IMGUI->Begin_TabItem("Human Point Cloud")) {
				TabIndex = 0;
				ObjPositionLimits.x = -10;
				ObjPositionLimits.y = 10;

				PLDCs.clear();
				GFX_API::PointLineDrawCall LDC;
				LDC.Draw_asPoint = true;
				LDC.PointBuffer_ID = OriginalHuman_PC;
				//LDC.ShaderInstance_ID = TuranEditor::RenderDataManager::NormalLine_MatInst;
				PLDCs.push_back(LDC);
				GFX_API::PointLineDrawCall PDC;
				PDC.Draw_asPoint = true;
				PDC.PointBuffer_ID = OriginalHuman_PC;
				//PDC.ShaderInstance_ID = TuranEditor::RenderDataManager::ShadedPoint_MatInst;
				PLDCs.push_back(PDC);

				{
					GFX_API::PointLineDrawCall LODC;
					LODC.Draw_asPoint = true;
					LODC.PointBuffer_ID = OriginalHuman_PC;
					//LODC.ShaderInstance_ID = TuranEditor::RenderDataManager::NormalLine_MatInst;
					PLDCs.push_back(LODC);
				}
			}
		}
		else {
			if (IMGUI->Begin_TabItem("Original Human")) {
				TabIndex = 0;
				ObjPositionLimits.x = -10;
				ObjPositionLimits.y = 10;

				PLDCs.clear();
				GFX_API::PointLineDrawCall LDC;
				LDC.Draw_asPoint = true;
				LDC.PointBuffer_ID = OriginalHuman_PC;
				//LDC.ShaderInstance_ID = TuranEditor::RenderDataManager::NormalLine_MatInst;
				PLDCs.push_back(LDC);
				GFX_API::PointLineDrawCall PDC;
				PDC.Draw_asPoint = true;
				PDC.PointBuffer_ID = OriginalHuman_PC;
				//PDC.ShaderInstance_ID = TuranEditor::RenderDataManager::ShadedPoint_MatInst;
				PLDCs.push_back(PDC);



				IMGUI->End_TabItem();
			}
			if (IMGUI->Begin_TabItem("Robust Normal Human")) {
				TabIndex = 0;
				ObjPositionLimits.x = -10;
				ObjPositionLimits.y = 10;

				PLDCs.clear();
				GFX_API::PointLineDrawCall LDC;
				LDC.Draw_asPoint = true;
				LDC.PointBuffer_ID = RobustNormalHuman_PC;
				//LDC.ShaderInstance_ID = TuranEditor::RenderDataManager::NormalLine_MatInst;
				PLDCs.push_back(LDC);
				GFX_API::PointLineDrawCall PDC;
				PDC.Draw_asPoint = true;
				PDC.PointBuffer_ID = RobustNormalHuman_PC;
				//PDC.ShaderInstance_ID = TuranEditor::RenderDataManager::ShadedPoint_MatInst;
				PLDCs.push_back(PDC);


				IMGUI->End_TabItem();
			}
		}

		IMGUI->Slider_Float("Camera Speed", &Cameras[TabIndex].cameraSpeed_Base, 0.0f, 1.0f);
		IMGUI->Slider_Vec3("First Object World Position", &Positions[TabIndex], ObjPositionLimits.x, ObjPositionLimits.y);
		IMGUI->Slider_Vec3("First Object Rotation", &Rotations[TabIndex], -180, 180);


		for (unsigned int DCIndex = 0; DCIndex < PLDCs.size(); DCIndex++) {
			GameRenderGraph->Register_PointDrawCall(PLDCs[DCIndex]);
		}
		vec2 MousePos = IMGUI->GetMouseWindowPos() - IMGUI->GetItemWindowPos();
		DisplayTexture = GFXContentManager->Find_Framebuffer_byGFXID(((GFX_API::DrawPass*)GameRenderGraph->Get_RenderNodes()[0])->Get_FramebufferID())->BOUND_RTs[0].RT_ID;
		DisplayWidth = 960;	DisplayHeight = 540;
		IMGUI->Display_Texture(DisplayTexture, DisplayWidth, DisplayHeight, true);


		if (MousePos.x >= 0.0f && MousePos.y >= 0.0f && DisplayWidth - MousePos.x > 0.0f && DisplayHeight - MousePos.y > 0.0f &&
			GFX->IsMouse_Clicked(GFX_API::MOUSE_BUTTONs::MOUSE_RIGHT_CLICK)) {
			if (!isMoving) {
				LastMousePos = MousePos;
				isMoving = true;
			}
			else {
				LOG_STATUS(to_string(Cameras[TabIndex].Front_Vector.x));
			}

			vec2 MouseOffset = MousePos - LastMousePos;
			MouseOffset.y = -MouseOffset.y;
			//Sensitivity
			MouseOffset *= 0.1f;
			Yaw_Pitch[TabIndex] += MouseOffset;
			if (Yaw_Pitch[TabIndex].y > 89.0f) {
				Yaw_Pitch[TabIndex].y = 89.0f;
			}
			else if (Yaw_Pitch[TabIndex].y < -89.0f) {
				Yaw_Pitch[TabIndex].y = -89.0f;
			}

			LastMousePos = MousePos;
			Cameras[TabIndex].Update(Yaw_Pitch[TabIndex].x, Yaw_Pitch[TabIndex].y);
			TuranEditor::RenderDataManager::FrontVec = Cameras[TabIndex].Front_Vector;
		}
		else {
			isMoving = false;
		}
		TuranEditor::RenderDataManager::FirstObjectPosition = Positions[TabIndex];
		TuranEditor::RenderDataManager::FirstObjectRotation = Rotations[TabIndex];
		TuranEditor::RenderDataManager::CameraPos = Cameras[TabIndex].Position;
		TuranEditor::RenderDataManager::FrontVec = Cameras[TabIndex].Front_Vector;
		IMGUI->End_TabBar();
	}

	if (GameRenderGraph) {
		GFX->Register_RenderGraph(GameRenderGraph);
	}

	IMGUI->End_Window();
}



void Main_Menubar_of_Editor() {
	if (!IMGUI->Begin_Menubar()) {
		IMGUI->End_Menubar();
	}
	//Successfully created the main menu bar!

	if (IMGUI->Begin_Menu("View")) {

		IMGUI->End_Menu();
	}


	//End Main Menubar operations!
	IMGUI->End_Menubar();
}




Camera::Camera(vec3 Position, vec3 target) : Position(Position), Front_Vector(target) {

}

void Camera::Update(float yaw, float pitch) {
	//Camera Rotation
	vec3 world_up(0, 1, 0);

	//Camera Direction Update
	if (yaw != 0.0f || pitch != 0.0f) {
		Front_Vector.x = cos(radians(pitch)) * cos(radians(yaw));
		Front_Vector.y = sin(radians(pitch));
		Front_Vector.z = cos(radians(pitch)) * sin(radians(yaw));
	}



	Front_Vector = normalize(Front_Vector);
	vec3 Right_Vector = normalize(cross(world_up, Front_Vector));
	vec3 Up_Vector = normalize(cross(Front_Vector, Right_Vector));


	//Camera Position Update
	float camera_speed = TuranEditor::Editor_System::Get_DeltaTime() * cameraSpeed_Base;
	if (GFX->IsKey_Pressed(GFX_API::KEYBOARD_KEYs::KEYBOARD_W)) {
		Position += Front_Vector * camera_speed;
	}
	if (GFX->IsKey_Pressed(GFX_API::KEYBOARD_KEYs::KEYBOARD_S)) {
		Position -= Front_Vector * camera_speed;
	}

	if (GFX->IsKey_Pressed(GFX_API::KEYBOARD_KEYs::KEYBOARD_D)) {
		Position -= Right_Vector * camera_speed;
	}
	if (GFX->IsKey_Pressed(GFX_API::KEYBOARD_KEYs::KEYBOARD_A)) {
		Position += Right_Vector * camera_speed;
	}

	if (GFX->IsKey_Pressed(GFX_API::KEYBOARD_KEYs::KEYBOARD_NP_8)) {
		Position += Up_Vector * camera_speed;
	}
	if (GFX->IsKey_Pressed(GFX_API::KEYBOARD_KEYs::KEYBOARD_NP_2)) {
		Position -= Up_Vector * camera_speed;
	}




	//View matrix construction
	view_matrix = lookAt(Position, Front_Vector + Position, world_up);
}