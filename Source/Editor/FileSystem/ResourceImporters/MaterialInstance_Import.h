#pragma once
#include "Editor/Editor_Includes.h"
#include "GFX/IMGUI/IMGUI_WINDOW.h"
#include "Editor/FileSystem/ResourceTypes/Resource_Identifier.h"

namespace TuranEditor {

	class MaterialInstance_CreationWindow : public GFX_API::IMGUI_WINDOW {
		Resource_Identifier* selected_materialtype = nullptr;
		string OUTPUT_FOLDER, OUTPUT_NAME;
		unsigned int selected_list_item_index = 0;
		vector<Resource_Identifier*> ALL_MATTYPEs;
		vector<string> item_names;
	public:
		MaterialInstance_CreationWindow();
		virtual void Run_Window() override;
	};

}