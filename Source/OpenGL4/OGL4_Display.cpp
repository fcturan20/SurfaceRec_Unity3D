#include "OGL4_Display.h"


namespace OpenGL4 {

	WINDOW::WINDOW(unsigned int width, unsigned int height, GFX_API::WINDOW_MODE display_mode, GFX_API::MONITOR* display_monitor
		, unsigned int refresh_rate, const char* window_name, GFX_API::V_SYNC v_sync)
	{
		WIDTH = width;
		HEIGHT = height;
		DISPLAY_MODE = display_mode;
		DISPLAY_MONITOR = display_monitor;
		REFRESH_RATE = refresh_rate;
		WINDOW_NAME = window_name;
		VSYNC_MODE = v_sync;
	}

}