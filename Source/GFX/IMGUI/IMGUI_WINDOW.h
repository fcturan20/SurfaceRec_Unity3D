#pragma once
#include "GFX/GFX_Includes.h"

namespace GFX_API {
	class GFXAPI IMGUI_WINDOW
	{
	protected:
		friend class IMGUI_WindowManager;
		bool Is_Window_Open = true;
		string Window_Name;

		//Definition of window's running progress!
		virtual void Run_Window() = 0;
	public:
		IMGUI_WINDOW(const char* name);


		bool Get_Is_Window_Open();

		virtual ~IMGUI_WINDOW();
	};
	class GFXAPI IMGUI_WindowManager {
	public:
		vector<IMGUI_WINDOW*> ALL_IMGUI_WINDOWs, Windows_toClose, Windows_toOpen;
		IMGUI_WindowManager();
		~IMGUI_WindowManager();
		//Run all of the active IMGUI windows!
		//But generally, if a window is out of view, it isn't runned. But this is defined in Run_Window() for each window!
		void Run_IMGUI_WINDOWs();
		void Register_WINDOW(IMGUI_WINDOW* WINDOW);
		void Delete_WINDOW(IMGUI_WINDOW* WINDOW);
	};
}