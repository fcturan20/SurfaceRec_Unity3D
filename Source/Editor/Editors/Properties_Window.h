#pragma once
#include "Editor/Editor_Includes.h"
#include "GFX/IMGUI/IMGUI_WINDOW.h"
#include "Editor/FileSystem/ResourceTypes/Resource_Identifier.h"
#include "GFX/Renderer/GFX_RenderGraph.h"

namespace TuranEditor {
	class ResourceProperties_Window : public GFX_API::IMGUI_WINDOW {
		bool changed_properties = false;
		unsigned int selected_texturechannelindex = 0;

		void Show_ShaderSource_Properties();
		void Show_MaterialType_Properties();
		void Show_Model_Properties();
		void Show_MaterialInstance_Properties();
		void Show_Texture_Properties();
	public:
		Resource_Identifier* RESOURCE = nullptr;
		GFX_API::RenderGraph* RenderGraph = nullptr;

		ResourceProperties_Window(Resource_Identifier* resource);
		virtual void Run_Window();

	};
}