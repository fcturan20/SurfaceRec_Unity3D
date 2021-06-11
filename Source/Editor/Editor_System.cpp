#include "Editor.h"
#include "OpenGL4/OGL4_Core.h"

#include "RenderContext/Editor_DataManager.h"

namespace TuranEditor {
	Editor_System::Editor_System() {
		new OpenGL4::OpenGL4_Core;
		RenderDataManager::Start_RenderingDataManagement();
	}
	Editor_System::~Editor_System() {
		((OpenGL4::OpenGL4_Core*)GFX)->~OpenGL4_Core();
	}
	void Editor_System::Take_Inputs() {
		GFX->Take_Inputs();
	}
	bool Editor_System::Should_EditorClose() {
		return Should_Close;
	}
	float Editor_System::Get_DeltaTime() {
		return DeltaTime_inmilliseconds;
	}
	void Editor_System::Set_DeltaTime(float delta) {
		DeltaTime_inmilliseconds = delta;
	}
	bool Editor_System::Should_Close = false;
	float Editor_System::DeltaTime_inmilliseconds = 16.66f;
}
