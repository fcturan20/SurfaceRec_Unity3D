#include "Editor.h"
using namespace TuranEditor;

int main() {
	Editor_System* EDITORSYSTEM = new Editor_System;
	Game_RenderGraph First_RenderGraph;


	//Main_Window* main_window = new Main_Window(PositionNormal_VertexAttrib, &First_RenderGraph);
	PCViewer* pcviewer = new PCViewer(&First_RenderGraph);
	long long FrameTime = 0;
	while (IMGUI->Is_IMGUI_Open) {
		FrameTime /= 1000;	//Convert from macroseconds to milliseconds
		Editor_System::Set_DeltaTime(FrameTime);
		LOG_STATUS("Delta time in float: " + to_string(float(FrameTime)));
		TURAN_PROFILE_SCOPE_O("Run Loop", &FrameTime);

		RenderDataManager::Update_GPUResources();
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