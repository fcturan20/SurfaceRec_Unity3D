#include "GFX_Core.h"
#include "TuranAPI/Logger_Core.h"

namespace GFX_API {

	GFX_Core* GFX_Core::SELF = nullptr;
	GFX_Core::GFX_Core() : RENDERER(nullptr),
		FOCUSED_WINDOW_index(0), GPU_TO_RENDER(nullptr) {
		LOG_STATUS("GFX Core systems are starting!");

		GFX = this;

		IMGUI = new GFX_API::IMGUI_Core;

		LOG_STATUS("GFX Core systems are started!");
	}
	GFX_Core::~GFX_Core() {
		std::cout << "GFX_Core's destructor is called!\n";
	}

	//Renderer Operations
	void GFX_Core::Register_RenderGraph(RenderGraph* RG) {
		RenderGraphs.push_back(RG);
	}
	bool GFX_Core::IsKey_Pressed(GFX_API::KEYBOARD_KEYs key) {
		return IO.isKeyPressed[GetKeyIndex(key)];
	}
	bool GFX_Core::IsMouse_Clicked(GFX_API::MOUSE_BUTTONs button) {
		return IO.isMouseClicked[GetMouseButtonIndex(button)];
	}


	MONITOR* GFX_Core::Create_MonitorOBJ(void* monitor, const char* name) { return new MONITOR(monitor, name); LOG_STATUS("A monitor is added"); }

}
