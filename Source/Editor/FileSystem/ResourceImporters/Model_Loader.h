#pragma once
#include "Editor/Editor_Includes.h"
#include "GFX/IMGUI/IMGUI_WINDOW.h"
#include "Editor/FileSystem/ResourceTypes/Model_Resource.h"
#include "GFX/GFX_FileSystem/Resource_Type/Material_Type_Resource.h"

namespace TuranEditor {

	class Model_Importer {
	public:
		static unsigned int Import_Model(const char* PATH, bool Use_SurfaceMaterial = true, unsigned int MaterialType_toInstance = 0);
	};

}