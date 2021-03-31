#include "GFX_Display.h"
#include "TuranAPI/Logger_Core.h"

namespace GFX_API {
	void MONITOR::Set_Physical_Size(unsigned int physical_width, unsigned int physical_height) {
		PHYSICAL_WIDTH = physical_width;
		PHYSICAL_HEIGHT = physical_height;
	}

	MONITOR::MONITOR(void* monitor, const char* name) : ID(monitor), NAME(name) {

	}

	const char* MONITOR::Get_Monitor_Name() {
		return NAME.c_str();
	}

	//Return WIDTH (x), HEIGHT (y) and REFRESH_RATE (<) as one vec3
	vec3 MONITOR::Get_Monitor_VidMode() {
		return vec3(WIDTH, HEIGHT, REFRESH_RATE);
	}

	void MONITOR::Set_Monitor_VidMode(unsigned int width, unsigned int height, unsigned int color_bites, unsigned int refrest_rate) {
		WIDTH = width;
		HEIGHT = height;
		COLOR_BITES = color_bites;
		REFRESH_RATE = refrest_rate;
		LOG_STATUS("Monitor's Video Mode is set!");
	}
	MONITOR& MONITOR::operator= (const MONITOR& from) {
		ID = from.ID;
		NAME = from.NAME;
		WIDTH = from.WIDTH; HEIGHT = from.HEIGHT; COLOR_BITES = from.COLOR_BITES; REFRESH_RATE = from.REFRESH_RATE;
		PHYSICAL_WIDTH = from.PHYSICAL_WIDTH; PHYSICAL_HEIGHT = from.PHYSICAL_HEIGHT;	//milimeters
		DESKTOP_MODE = from.DESKTOP_MODE;
		return *this;
	}


	WINDOW::WINDOW(){}


	const char* WINDOW::Get_Window_Name() const {
		return WINDOW_NAME.c_str();
	}

	//Return WIDTH (x), HEIGHT (y) and REFRESH_RATE (z) as one vec3
	vec3 WINDOW::Get_Window_Mode() {
		return vec3(WIDTH, HEIGHT, REFRESH_RATE);
	}

	//	1) Change the window's display mode (Fullscreen, Windowed, Borderless Window etc.)
	//	2) Or change to intended refresh rate or monitor
	//	3) No resolution changing in this function, window's active resolution is used
	void WINDOW::Change_DisplayMode(WINDOW_MODE display_mode, const MONITOR* display_monitor, unsigned int refresh_rate) {}

	void WINDOW::Set_Focus(bool focus_active) {
		std::cout << "Error: Set focus isn't coded, don't use it!\n";
	}

	//Settings of window
	void WINDOW::Window_Settings(V_SYNC vsync_type, bool window_resizable) {
		LOG_NOTCODED("WINDOW::Window_Settings isn't coded!", false);
	}

	void WINDOW::Change_Width_Height(unsigned int width, unsigned int height) {
		WIDTH = width;
		HEIGHT = height;
		LOG_STATUS("Window's width and height is changed!");
	}
}