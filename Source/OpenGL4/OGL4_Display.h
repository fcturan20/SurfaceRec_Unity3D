#pragma once
#include "OpenGL4_Includes.h"
#include "GFX/GFX_Display.h"
#include "GFX/GFX_ENUMs.h"


namespace OpenGL4 {
	class OGL4_API WINDOW : public GFX_API::WINDOW {
	public:
		WINDOW(unsigned int width, unsigned int height, GFX_API::WINDOW_MODE display_mode, GFX_API::MONITOR* display_monitor, unsigned int refresh_rate, const char* window_name, GFX_API::V_SYNC v_sync);
		GLFWwindow* GLFWWINDOW = {};
	};
}