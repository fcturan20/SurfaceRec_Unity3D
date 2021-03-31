#pragma once
#include "Editor/Editor_Includes.h"
#include "GFX/IMGUI/IMGUI_WINDOW.h"

namespace TuranEditor {

	class Status_Window : public GFX_API::IMGUI_WINDOW {
	public:
		Status_Window(const char* error);
		virtual void Run_Window();

		string STATUS;
	};

}