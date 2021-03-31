#include "Main_Window.h"

#include "GFX/GFX_Core.h"
#include "Editor/RenderContext/Game_RenderGraph.h"
#include "GFX/GFX_Core.h"
#include "Editor/RenderContext/Editor_DataManager.h"
#include "Editor/Editor.h"
#include <string>

using namespace TuranAPI;

namespace TuranEditor {
	Main_Window::Main_Window(GFX_API::VertexAttributeLayout& PositionNormal_VertexAttrib, Game_RenderGraph* game_rendergraph) : IMGUI_WINDOW("Main_Window"), 
		GameRenderGraph(game_rendergraph), VAL(PositionNormal_VertexAttrib), 
		SurfaceMatInst(Editor_RendererDataManager::Create_SurfaceMaterialInstance(SURFACEMAT_PROPERTIES(), new unsigned int(0))){
		IMGUI_REGISTERWINDOW(this);

		//Import original models with indexes
		const char* CAT = "C:/Users/furka/Desktop/Meshes/cat0.off";
		const char* HUMAN = "C:/Users/furka/Desktop/Meshes/SCAPE/Meshes/mesh000.off";
		unsigned int CAT_ASSETID = Model_Importer::Import_Model(CAT);
		unsigned int HUMAN_ASSETID = Model_Importer::Import_Model(HUMAN);

		if (!CAT_ASSETID || !HUMAN_ASSETID) {
			LOG_WARNING("Failed to import Original objects!\n");
		}

		//Send original models with indexes to GPU 
		Static_Model* CAT_MODEL = (Static_Model*)EDITOR_FILESYSTEM->Find_ResourceIdentifier_byID(CAT_ASSETID)->DATA;
		Static_Model* HUMAN_MODEL = (Static_Model*)EDITOR_FILESYSTEM->Find_ResourceIdentifier_byID(HUMAN_ASSETID)->DATA;
		for (unsigned int i = 0; i < CAT_MODEL->MESHes.size(); i++) {
			Static_Mesh_Data* MESH = CAT_MODEL->MESHes[i];
			CatOriginal.push_back(GFXContentManager->Upload_MeshBuffer(MESH->DataLayout, MESH->VERTEX_DATA, MESH->VERTEXDATA_SIZE, MESH->VERTEX_NUMBER, 
				MESH->INDEX_DATA, MESH->INDICES_NUMBER));
		}
		for (unsigned int i = 0; i < HUMAN_MODEL->MESHes.size(); i++) {
			Static_Mesh_Data* MESH = HUMAN_MODEL->MESHes[i];
			HumanOriginal.push_back(GFXContentManager->Upload_MeshBuffer(MESH->DataLayout, MESH->VERTEX_DATA, MESH->VERTEXDATA_SIZE, MESH->VERTEX_NUMBER,
				MESH->INDEX_DATA, MESH->INDICES_NUMBER));
		}


		//Import same models as point clouds and reconstruct their surfaces
		CatPC = DataLoader::LoadMesh_asPointCloud(CAT);
		HumanPC = DataLoader::LoadMesh_asPointCloud(HUMAN);
		if (!CatPC || !HumanPC) {
			LOG_CRASHING("Point Cloud import has failed!");
		}
		HumanReconstructed = Algorithms::ReconstructSurface(HumanPC, HumanK, PositionNormal_VertexAttrib);
		CatReconstructed = Algorithms::ReconstructSurface(CatPC, CatK, PositionNormal_VertexAttrib);
	}

	//Create main menubar for the Editor's main window!
	void Main_Menubar_of_Editor();

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

		if (IMGUI->Begin_TabBar()) {
			if (IMGUI->Begin_TabItem("Lights")) {
				IMGUI->Slider_Vec3("Directional Light Color", (vec3*)&Editor_RendererDataManager::DIRECTIONALLIGHTs[0].COLOR, 0, 1);
				IMGUI->Slider_Vec3("Directional Light Direction", (vec3*)&Editor_RendererDataManager::DIRECTIONALLIGHTs[0].DIRECTION, -1, 1);
				IMGUI->End_TabItem();
			}
			ivec2 ObjPositionLimits;
			if (IMGUI->Begin_TabItem("Original Human")) {
				TabIndex = 0;
				ObjPositionLimits.x = -10;
				ObjPositionLimits.y = 10;

				DCs.clear();
				for (unsigned int MeshIndex = 0; MeshIndex < HumanOriginal.size(); MeshIndex++) {
					GFX_API::DrawCall DC;
					DC.MeshBuffer_ID = HumanOriginal[MeshIndex];
					DC.ShaderInstance_ID = SurfaceMatInst;
					DC.JoinedDrawPasses = 0xffffffff;	//Join all draw passes for now!
					DCs.push_back(DC);
				}

				IMGUI->End_TabItem();
			}
			if (IMGUI->Begin_TabItem("Reconstructed Human")) {
				TabIndex = 0;
				ObjPositionLimits.x = -10;
				ObjPositionLimits.y = 10;
				int NewK = HumanK;
				if (IMGUI->Slider_Int("K-Nearest: ", &NewK, 3, 30)) {
					if (NewK != HumanK) {
						GFXContentManager->Unload_MeshBuffer(HumanReconstructed);
						HumanK = NewK;
						HumanReconstructed = Algorithms::ReconstructSurface(HumanPC, HumanK, VAL);
					}
				}

				DCs.clear();
				GFX_API::DrawCall DC;
				DC.MeshBuffer_ID = HumanReconstructed;
				DC.JoinedDrawPasses = 0xFF;
				DC.ShaderInstance_ID = SurfaceMatInst;
				DCs.push_back(DC);


				IMGUI->End_TabItem();
			}
			if (IMGUI->Begin_TabItem("Original Cat")) {
				TabIndex = 1;
				ObjPositionLimits.x = -500;
				ObjPositionLimits.y = 500;

				DCs.clear();
				for (unsigned int MeshIndex = 0; MeshIndex < CatOriginal.size(); MeshIndex++) {
					GFX_API::DrawCall DC;
					DC.MeshBuffer_ID = CatOriginal[MeshIndex];
					DC.ShaderInstance_ID = SurfaceMatInst;
					DC.JoinedDrawPasses = 0xffffffff;	//Join all draw passes for now!
					DCs.push_back(DC);
				}

				IMGUI->End_TabItem();
			}
			if (IMGUI->Begin_TabItem("Reconstructed Cat")) {
				TabIndex = 1;
				ObjPositionLimits.x = -500;
				ObjPositionLimits.y = 500;

				int NewK = CatK;
				if (IMGUI->Slider_Int("K-Nearest: ", &NewK, 3, 30)) {
					if (NewK != CatK) {
						GFXContentManager->Unload_MeshBuffer(CatReconstructed);
						CatK = NewK;
						CatReconstructed = Algorithms::ReconstructSurface(CatPC, CatK, VAL);
					}
				}

				DCs.clear();
				GFX_API::DrawCall DC;
				DC.MeshBuffer_ID = CatReconstructed;
				DC.JoinedDrawPasses = 0xFF;
				DC.ShaderInstance_ID = SurfaceMatInst;
				DCs.push_back(DC);

				IMGUI->End_TabItem();
			}
			IMGUI->Slider_Float("Camera Speed", &Cameras[TabIndex].cameraSpeed_Base, 0.0f, 1.0f);
			IMGUI->Slider_Vec3("First Object World Position", &Positions[TabIndex], ObjPositionLimits.x, ObjPositionLimits.y);
			IMGUI->Slider_Vec3("First Object Rotation", &Rotations[TabIndex], -180, 180);


			for (unsigned int DCIndex = 0; DCIndex < DCs.size(); DCIndex++) {
				GameRenderGraph->Register_DrawCall(DCs[DCIndex]);
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
			}
			else {
				isMoving = false;
			}
			Editor_RendererDataManager::FirstObjectPosition = Positions[TabIndex];
			Editor_RendererDataManager::FirstObjectRotation = Rotations[TabIndex];
			TuranEditor::Editor_RendererDataManager::CameraPos = Cameras[TabIndex].Position;
			TuranEditor::Editor_RendererDataManager::FrontVec = Cameras[TabIndex].Front_Vector;
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




	Camera::Camera(vec3 Position, vec3 target) : Position(Position), target(target) {
		Front_Vector = normalize(target - Position);
	}

	void Camera::Update(float yaw, float pitch) {
		//Camera Rotation
		vec3 world_up(0, 1, 0);

		//Camera Direction Update
		if (yaw != 0.0f || pitch != 0.0f) {
			target.x = cos(radians(pitch)) * cos(radians(yaw));
			target.y = sin(radians(pitch));
			target.z = cos(radians(pitch)) * sin(radians(yaw));
		}



		Front_Vector = normalize(target);
		Right_Vector = normalize(cross(world_up, Front_Vector));
		Up_Vector = normalize(cross(Front_Vector, Right_Vector));


		//Camera Position Update
		float camera_speed =  Editor_System::Get_DeltaTime() * cameraSpeed_Base;
		if (GFX->IsKey_Pressed(GFX_API::KEYBOARD_KEYs::KEYBOARD_W)) {
			Position += Front_Vector * camera_speed;
		}
		else if (GFX->IsKey_Pressed(GFX_API::KEYBOARD_KEYs::KEYBOARD_S)) {
			Position -= Front_Vector * camera_speed;
		}

		if (GFX->IsKey_Pressed(GFX_API::KEYBOARD_KEYs::KEYBOARD_D)) {
			Position -= Right_Vector * camera_speed;
		}
		else if (GFX->IsKey_Pressed(GFX_API::KEYBOARD_KEYs::KEYBOARD_A)) {
			Position += Right_Vector * camera_speed;
		}

		if (GFX->IsKey_Pressed(GFX_API::KEYBOARD_KEYs::KEYBOARD_NP_8)) {
			Position += Up_Vector * camera_speed;
		}
		else if (GFX->IsKey_Pressed(GFX_API::KEYBOARD_KEYs::KEYBOARD_NP_2)) {
			Position -= Up_Vector * camera_speed;
		}




		//View matrix construction
		view_matrix = lookAt(Position, Front_Vector + Position, world_up);
	}
}