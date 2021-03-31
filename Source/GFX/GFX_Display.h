#pragma once
#include "GFX_Includes.h"
#include "GFX_ENUMs.h"

namespace GFX_API {
	//I have no function to do something on monitor, so this is a storage struct
	struct GFXAPI MONITOR {
		friend class GFX_Core;

		void* ID;
		string NAME;
		unsigned int WIDTH, HEIGHT, COLOR_BITES, REFRESH_RATE;
		int PHYSICAL_WIDTH, PHYSICAL_HEIGHT;	//milimeters
		WINDOW_MODE DESKTOP_MODE;

		void Set_Physical_Size(unsigned int physical_width, unsigned int physical_height);
		MONITOR(void* monitor = nullptr, const char* name = "");
	public:

		const char* Get_Monitor_Name();
		//Return WIDTH (x), HEIGHT (y) and REFRESH_RATE (<) as one vec3
		vec3 Get_Monitor_VidMode();
		MONITOR& operator= (const MONITOR& from);

		void Set_Monitor_VidMode(unsigned int width, unsigned int height, unsigned int color_bites, unsigned int refrest_rate);
	};



	//Note: Window ID created by GFX API specific library stored as void* WINDOW
	//If you want to access it, you want to access it with equal library class
	//Window resolution is independent from render resolution
	//After rendering has done, render resolution
	struct GFXAPI WINDOW {
	protected:
		friend class GFX_Core;

		unsigned int WIDTH, HEIGHT, REFRESH_RATE;
		MONITOR* DISPLAY_MONITOR;
		WINDOW_MODE DISPLAY_MODE;
		string WINDOW_NAME;
		V_SYNC VSYNC_MODE;

		WINDOW();
	public:
		//SETTERs

		//	1) Change the window's display mode (Fullscreen, Windowed, Borderless Window etc.)
		//	2) Or change to intended refresh rate or monitor
		//	3) No resolution changing in this function, window's active resolution is used
		void Change_DisplayMode(WINDOW_MODE display_mode, const MONITOR* display_monitor, unsigned int refresh_rate);
		void Set_Focus(bool focus_active);
		void Window_Settings(V_SYNC vsync_type, bool window_resizable);
		void Change_Width_Height(unsigned int width, unsigned int height);

		//GETTERs

		const char* Get_Window_Name() const;

		//Return WIDTH (x), HEIGHT (y) and REFRESH_RATE (<) as one vec3
		vec3 Get_Window_Mode();
	};


	struct GFXAPI GPU {
	public:
		string MODEL;
		uint32_t API_VERSION;
		uint32_t DRIVER_VERSION;
		GPU_TYPEs GPU_TYPE;
		bool is_GraphicOperations_Supported = false, is_ComputeOperations_Supported = false, is_TransferOperations_Supported = false, is_DisplayOperations_Supported;
	};
}