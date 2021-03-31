#include "Status_Window.h"
#include "GFX/GFX_Core.h"

using namespace GFX_API;

namespace TuranEditor {

	Status_Window::Status_Window(const char* status) : IMGUI_WINDOW("Status Window"), STATUS(status) {
		IMGUI_REGISTERWINDOW(this);
	}

	void Status_Window::Run_Window() {
		if (!Is_Window_Open) {
			IMGUI_DELETEWINDOW(this);
			return;
		}
		if (!IMGUI->Create_Window("Status Window", Is_Window_Open, false)) {
			IMGUI->End_Window();
			return;
		}
		//Successfully created here!
		IMGUI->Text(STATUS.c_str());

		IMGUI->End_Window();
	}

}