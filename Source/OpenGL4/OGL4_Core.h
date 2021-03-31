#pragma once
#include "OGL4_ENUMs.h"
#include "Renderer/OGL4_Renderer_Core.h"

#include "OGL4_Display.h"

namespace OpenGL4 {
	class OGL4_API OpenGL4_Core : public GFX_API::GFX_Core {


		virtual void Initialization() override;
		virtual void Check_Computer_Specs() override;
		virtual void Save_Monitors() override;
		virtual void Create_Renderer() override;
		void Create_MainWindow();
		static void GFX_Error_Callback(int error_code, const char* description);
	public:
		OpenGL4_Core();
		virtual ~OpenGL4_Core();

		//Window Operations
		virtual void Change_Window_Resolution(unsigned int width, unsigned int height) override;
		virtual void Show_RenderTarget_onWindow(unsigned int RenderTarget_GFXID) override;

		//Window Callbacks
		static void window_focus_callback(GLFWwindow* window, int focused);
		static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
		static void window_close_callback(GLFWwindow* window);

		//Renderer Operations
		virtual void Check_Errors() override;
		virtual void Swapbuffers_ofMainWindow() override;
		virtual void Run_RenderGraphs() override;

		//Input (Keyboard-Controller) Operations
		virtual void Take_Inputs() override;
		static void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void MouseCallback(GLFWwindow* window, int key, int action, int mods);

		//Resource Destroy Operations
		virtual void Destroy_GFX_Resources() override;
	};

}