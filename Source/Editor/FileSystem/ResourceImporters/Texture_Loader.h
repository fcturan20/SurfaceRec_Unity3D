#pragma once
#include "Editor/Editor_Includes.h"
#include "GFX/IMGUI/IMGUI_WINDOW.h"
#include "Editor/FileSystem/ResourceTypes/Resource_Identifier.h"
#include "GFX/Renderer/GFX_Resource.h"

namespace TuranEditor {
	class Texture_Loader {
	public:
		static Resource_Identifier* Import_Texture(const char* PATH,
			const bool& flip_vertically = false, const GFX_API::Texture_Properties& Properties = GFX_API::Texture_Properties());
	};

}