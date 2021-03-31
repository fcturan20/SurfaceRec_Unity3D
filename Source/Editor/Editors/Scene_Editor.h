#pragma once
#include "Editor/Editor_Includes.h"
#include "GFX/IMGUI/IMGUI_WINDOW.h"
#include "GFX/Renderer/GFX_RenderGraph.h"
#include "Editor/FileSystem/ResourceTypes/Resource_Identifier.h"

namespace TuranEditor {

	class Scene_Editor : public GFX_API::IMGUI_WINDOW {
		Resource_Identifier* SCENE_to_EDIT;
		GFX_API::RenderGraph* RenderGraph_forScene;
		vector<string> item_names, component_names;
	public:
		Scene_Editor(Resource_Identifier* SCENE);
		virtual void Run_Window() override;
	};

}