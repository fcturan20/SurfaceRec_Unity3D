#pragma once
#include "GFX_Includes.h"
#include "GFX_Display.h"
#include "Renderer/GFX_Renderer.h"
#include "Renderer/GPU_ContentManager.h"
#include "IMGUI/IMGUI_Core.h"


namespace GFX_API {
	class GFXAPI GFX_Core {
	protected:
		unsigned int FOCUSED_WINDOW_index;
		vector<GPU*> DEVICE_GPUs;
		//This GPU will be used to render! There is no multi-GPU support for now
		GPU* GPU_TO_RENDER;
		vector<RenderGraph*> RenderGraphs;
		InputHandler IO;

		virtual void Initialization() = 0;
		virtual void Check_Computer_Specs() = 0;
		virtual void Save_Monitors() = 0;
		virtual void Create_Renderer() = 0;
		GFX_Core();
		virtual ~GFX_Core();

		MONITOR* Create_MonitorOBJ(void* monitor, const char* name);
	public:
		static GFX_Core* SELF;
		Renderer* RENDERER;
		GPU_ContentManager* ContentManager;
		IMGUI_Core* IMGUI_o;

		WINDOW* Main_Window;
		vector<MONITOR> CONNECTED_Monitors;

		//Window Operations

		virtual void Change_Window_Resolution(unsigned int width, unsigned int height) = 0;
		virtual void Swapbuffers_ofMainWindow() = 0;
		virtual void Show_RenderTarget_onWindow(unsigned int RenderTarget_GFXID) = 0;

		//Renderer Operations

		//Each frame, you should call this to register your RG to the "RenderGraphsthatWillBeRenderedThisFrame" list
		void Register_RenderGraph(RenderGraph* RG);
		//Each frame, you should call this to render your RenderGraphs
		virtual void Run_RenderGraphs() = 0;

		virtual void Take_Inputs() = 0;
		bool IsKey_Pressed(KEYBOARD_KEYs key);
		bool IsMouse_Clicked(MOUSE_BUTTONs button);

		virtual void Destroy_GFX_Resources() = 0;
		virtual void Check_Errors() = 0;
	};

}

#define GFX GFX_API::GFX_Core::SELF
#define GFXRENDERER GFX_API::GFX_Core::SELF->RENDERER
#define GFXContentManager GFX_API::GFX_Core::SELF->ContentManager
#define IMGUI GFX_API::GFX_Core::SELF->IMGUI_o

	//Window Manager functionality!
#define IMGUI_REGISTERWINDOW(WINDOW) GFX_API::GFX_Core::SELF->IMGUI->WindowManager->Register_WINDOW(WINDOW);
#define IMGUI_DELETEWINDOW(WINDOW) GFX_API::GFX_Core::SELF->IMGUI->WindowManager->Delete_WINDOW(WINDOW);
#define IMGUI_RUNWINDOWS() GFX_API::GFX_Core::SELF->IMGUI->WindowManager->Run_IMGUI_WINDOWs()