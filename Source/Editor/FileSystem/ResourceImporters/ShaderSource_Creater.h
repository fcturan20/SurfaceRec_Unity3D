#pragma once
#include "Editor/Editor_Includes.h"
#include "Editor/FileSystem/EditorFileSystem_Core.h"
#include "GFX/GFX_FileSystem/Resource_Type/Material_Type_Resource.h"
#include "GFX/IMGUI/IMGUI_WINDOW.h"


namespace TuranEditor {
	class ShaderSource_CreationWindow : public GFX_API::IMGUI_WINDOW {
		GFX_API::SHADER_STAGE SHADERSTAGE;
		GFX_API::SHADER_LANGUAGEs LANGUAGE;
		string CODE, SOURCECODE_PATH, OUTPUT_FOLDER, OUTPUT_NAME;
		unsigned int selected_shaderstageindex, selected_shaderlanguageindex;
	public:
		ShaderSource_CreationWindow();
		virtual void Run_Window() override;
	};
}
