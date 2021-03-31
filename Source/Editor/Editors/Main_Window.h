#pragma once
#include "Editor/Editor_Includes.h"
#include "GFX/IMGUI/IMGUI_WINDOW.h"
#include "Editor/RenderContext/Game_RenderGraph.h"

namespace TuranEditor {
	class Camera {
	public:
		Camera(vec3 Position = vec3(0, 0, 0), vec3 target = vec3(0, 0, 1));
		void Update(float yaw, float pitch);
		vec3 Position, Front_Vector, Right_Vector, Up_Vector, target;
		mat4 view_matrix;
		float cameraSpeed_Base = 1.0f;
	};

	class Main_Window : public GFX_API::IMGUI_WINDOW {
		unsigned int DisplayTexture = 0, DisplayWidth = 0, DisplayHeight = 0, Scene_View_Texture = 0;
		Game_RenderGraph* GameRenderGraph = nullptr;
		//Camera related data
		Camera DrawPassCamera;
		vec2 LastMousePos = vec2(0, 0), Yaw_Pitch = vec2(89.0f,0);
		bool isMoving = false;
	public:
		Main_Window(Game_RenderGraph* GameRenderGraph = nullptr);
		virtual void Run_Window();
	};

}