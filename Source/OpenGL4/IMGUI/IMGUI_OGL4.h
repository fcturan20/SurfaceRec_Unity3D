#pragma once
#include "TuranAPI/API_includes.h"
#include "GFX/IMGUI/IMGUI_GFX.h"

namespace OpenGL4 {
	class IMGUI_OGL4 : public GFX_API::IMGUI_GFX {
	public:
		virtual void Initialize(void* Window_GPU_Context);
		virtual void Render_IMGUI(void* data);
		virtual void GFX_New_Frame();
		virtual void Destroy_IMGUI_GFX_Resources();
		virtual void Set_Platform_Settings();
	};
}