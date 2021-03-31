#pragma once
#include "Editor/Editor_Includes.h"
#include "GFX/IMGUI/IMGUI_WINDOW.h"
#include "Editor/RenderContext/Game_RenderGraph.h"
#include "GFX/GFX_Core.h"
#include "Editor/Ýþ/DataTypes.h"

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
		unsigned int DisplayTexture = 0, DisplayWidth = 0, DisplayHeight = 0, Scene_View_Texture = 0, SurfaceMatInst;
		GFX_API::VertexAttributeLayout& VAL;
		Game_RenderGraph* GameRenderGraph = nullptr;
		vector<GFX_API::DrawCall> DCs;

		PointCloud* HumanPC = nullptr, *CatPC = nullptr;
		unsigned int HumanReconstructed = UINT32_MAX, CatReconstructed = UINT32_MAX, HumanK = 3, CatK = 3;
		vector<unsigned int> CatOriginal, HumanOriginal;

		vec2 LastMousePos = vec2(0, 0);
		bool isMoving = false;

		//Tab (View) related data
		Camera Cameras[2];
		vec3 Positions[2]; vec3 Rotations[2]; vec2 Yaw_Pitch[2]{ vec2(90.0f, 0) };
		unsigned char TabIndex = 0;
	public:
		Main_Window(GFX_API::VertexAttributeLayout& PositionNormal_VertexAttrib, Game_RenderGraph* GameRenderGraph = nullptr);
		virtual void Run_Window();
	};

}