#include "Scene_Editor.h"
#include "Editor/FileSystem/EditorFileSystem_Core.h"
#include "Editor/FileSystem/ResourceTypes/ResourceTYPEs.h"

#include "Editor/FileSystem/ResourceTypes/Scene_Resource.h"
#include "GFX/GFX_Core.h"
#include "Editor/RenderContext/Game_RenderGraph.h"

#include "Status_Window.h"
using namespace TuranAPI;
using namespace GFX_API;


namespace TuranEditor {
	Scene_Editor::Scene_Editor(Resource_Identifier* SCENE) : IMGUI_WINDOW("Scene Editor"), SCENE_to_EDIT(SCENE) {
		if (!SCENE) {
			LOG_CRASHING("Scene_Resource* that's given to Scene_Editor is nullptr!\n");
			return;
		}
		IMGUI_REGISTERWINDOW(this);
	}

	void Scene_Editor::Run_Window() {
		if (!Is_Window_Open) {
			IMGUI_DELETEWINDOW(this);
			return;
		}
		if (!IMGUI->Create_Window(Window_Name.c_str(), Is_Window_Open, true)) {
			IMGUI->End_Window();
			return;
		}

		//Even if there is a importing proccess, show the contents!
		int selected_list_item_index = -1;
		

		Scene_Resource* SCENE_DATA = (Scene_Resource*)SCENE_to_EDIT->DATA;



		IMGUI->End_Window();
	}
}