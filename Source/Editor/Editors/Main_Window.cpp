#include "Main_Window.h"

#include "GFX/GFX_Core.h"
#include "Editor/RenderContext/Game_RenderGraph.h"
#include "GFX/GFX_Core.h"
#include "Editor/RenderContext/Editor_DataManager.h"
#include "Editor/RenderContext/Compute Passes/FirstCompute.h"
#include "Editor/Editor.h"
#include <string>

using namespace TuranAPI;

namespace TuranEditor {
	Main_Window::Main_Window(Game_RenderGraph* game_rendergraph) : IMGUI_WINDOW("Main_Window"), GameRenderGraph(game_rendergraph), DrawPassCamera(vec3(0,0,0), vec3(0,0,1)) {
		IMGUI_REGISTERWINDOW(this);
	}

	//Create main menubar for the Editor's main window!
	void Main_Menubar_of_Editor();

	void Main_Window::Run_Window() {
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
			if (IMGUI->Begin_TabItem("Draw Pass")) {
				IMGUI->Slider_Float("Camera Speed", &DrawPassCamera.cameraSpeed_Base, 0.0f, 1.0f);
				IMGUI->Slider_Vec3("First Object World Position", &Editor_RendererDataManager::FirstObjectPosition, -500, 500);
				IMGUI->Slider_Vec3("First Object Rotation", &Editor_RendererDataManager::FirstObjectRotation, -180, 180);
				//Display MainDrawPass's Color Buffer
				{
					DisplayTexture = GFXContentManager->Find_Framebuffer_byGFXID(((GFX_API::DrawPass*)GameRenderGraph->Get_RenderNodes()[0])->Get_FramebufferID())->BOUND_RTs[0].RT_ID;
					DisplayWidth = 960;	DisplayHeight = 540;
				}

				IMGUI->End_TabItem();
			}
			if (IMGUI->Begin_TabItem("Compute Pass")) {
				IMGUI->Slider_Vec3("Camera Position", &FirstCompute::RAYTRACECAMERA_POS, -100, 100);
				IMGUI->Slider_Vec3("Camera Front Vector", &FirstCompute::RAYTRACECAMERA_FRONTVECTOR, -1, 1);
				DisplayTexture = FirstCompute::OUTPUTTEXTURE_ID;
				DisplayWidth = 360; DisplayHeight = 360;
				IMGUI->End_TabItem();
			}
			if (DisplayTexture) {
				vec2 MousePos = IMGUI->GetMouseWindowPos() - IMGUI->GetItemWindowPos();
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
					Yaw_Pitch += MouseOffset;
					if (Yaw_Pitch.y > 89.0f) {
						Yaw_Pitch.y = 89.0f;
					}
					else if (Yaw_Pitch.y < -89.0f) {
						Yaw_Pitch.y = -89.0f;
					}

					LastMousePos = MousePos;
				}
				else {
					isMoving = false;
				}
				DrawPassCamera.Update(Yaw_Pitch.x, Yaw_Pitch.y);
			}
		}

		if (GameRenderGraph) {
			GFX->Register_RenderGraph(GameRenderGraph);
			//Display FirstCompute's Output Texture
			{

			}
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
		TuranEditor::Editor_RendererDataManager::CameraPos = Position;
		TuranEditor::Editor_RendererDataManager::FrontVec = Front_Vector;
	}
}