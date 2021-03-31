#include "MaterialInstance_Import.h"
#include "Editor/FileSystem/EditorFileSystem_Core.h"
#include "GFX/GFX_Core.h"
#include "TuranAPI/FileSystem_Core.h"
#include "Editor/FileSystem/ResourceTypes/ResourceTYPEs.h"

#include "Editor/Editors/Status_Window.h"

using namespace TuranAPI;
using namespace TuranEditor;

namespace TuranEditor {

	static void Create_Instance_ofMaterialType(Resource_Identifier* material_type, vector<GFX_API::Material_Uniform>& Uniform_List, const char* output_path);

	MaterialInstance_CreationWindow::MaterialInstance_CreationWindow() : IMGUI_WINDOW("Material Instance Import") {
		IMGUI_REGISTERWINDOW(this);
	}

	void MaterialInstance_CreationWindow::Run_Window() {
		if (!Is_Window_Open) {
			IMGUI_DELETEWINDOW(this);
			return;
		}
		if (!IMGUI->Create_Window(Window_Name.c_str(), Is_Window_Open, false)) {
			IMGUI->End_Window();
			return;
		}
		if (!EDITOR_FILESYSTEM->Get_SpecificAssetType(RESOURCETYPEs::GFXAPI_MATTYPE).size()) {
			IMGUI->Text("There is no Material Type, so you can't create  a Material Instance!");
			IMGUI->End_Window();
			return;
		}


		IMGUI->Input_Text("Output Folder", &OUTPUT_FOLDER);
		IMGUI->Input_Text("Output Name", &OUTPUT_NAME);
		string PATH = OUTPUT_FOLDER;
		PATH.append(OUTPUT_NAME);

		GFX_API::Material_Instance* created_matinst;
		vector<GFX_API::Material_Uniform> UNIFORM_LIST;


		if (IMGUI->Button("Create")) {
			selected_materialtype = ALL_MATTYPEs[selected_list_item_index];
			GFX_API::Material_Type* MATTYPE = (GFX_API::Material_Type*)selected_materialtype->DATA;
			UNIFORM_LIST.clear();
			for (unsigned int i = 0; i < MATTYPE->UNIFORMs.size(); i++) {
				UNIFORM_LIST.push_back(MATTYPE->UNIFORMs[i]);
			}
			Create_Instance_ofMaterialType(selected_materialtype, UNIFORM_LIST, PATH.c_str());
		}


		if (ALL_MATTYPEs.size() != EDITOR_FILESYSTEM->Get_SpecificAssetType(RESOURCETYPEs::GFXAPI_MATTYPE).size()) {
			ALL_MATTYPEs.clear(); item_names.clear();
			ALL_MATTYPEs = EDITOR_FILESYSTEM->Get_SpecificAssetType(RESOURCETYPEs::GFXAPI_MATTYPE);
			for (unsigned int i = 0; i < ALL_MATTYPEs.size(); i++) {
				item_names.push_back(ALL_MATTYPEs[i]->Get_Name());
			}
		}

		const char* nameof_selectlistline = "Material Type List";

		//Show Material Type list and if one of them is selected, create uniforms for the material instance
		GFX_API::Material_Uniform* UNIFORM = nullptr;
		IMGUI->SelectList_OneLine(nameof_selectlistline, &selected_list_item_index, &item_names);

		if (IMGUI->Begin_Tree("Uniform List")) {
			//A different Material Type
			if (selected_materialtype != ALL_MATTYPEs[selected_list_item_index]) {
				selected_materialtype = ALL_MATTYPEs[selected_list_item_index];
				GFX_API::Material_Type* MATTYPE = (GFX_API::Material_Type*)selected_materialtype->DATA;
				UNIFORM_LIST.clear();
				for (unsigned int i = 0; i < MATTYPE->UNIFORMs.size(); i++) {
					UNIFORM_LIST.push_back(MATTYPE->UNIFORMs[i]);
				}
			}

			for (unsigned int i = 0; i < UNIFORM_LIST.size(); i++) {
				UNIFORM = &UNIFORM_LIST[i];

				if (IMGUI->Begin_Tree(std::to_string(i).c_str())) {
					IMGUI->Text("I don't know what to show here!");

					IMGUI->End_Tree();
				}
			}
			IMGUI->End_Tree();
		}


		IMGUI->End_Window();
	}


	//Output Path should be Directory + Name. Like "C:/dev/Content/First". Every Material Instance has .matinstcont extension!
	void Create_Instance_ofMaterialType(Resource_Identifier* material_type, vector<GFX_API::Material_Uniform>& UNIFORM_LIST, const char* output_path) {
		GFX_API::Material_Instance* MATERIAL_INSTANCE = new GFX_API::Material_Instance;
		Resource_Identifier* RESOURCE = new Resource_Identifier;
		MATERIAL_INSTANCE->Material_Type = material_type->ID;

		GFX_API::Material_Type* MATTYPE = (GFX_API::Material_Type*)material_type->DATA;
		MATERIAL_INSTANCE->UNIFORM_LIST = MATTYPE->UNIFORMs;
		RESOURCE->PATH = output_path;
		RESOURCE->PATH.append(".matinstcont");
		RESOURCE->DATA = MATERIAL_INSTANCE;
		RESOURCE->TYPE = RESOURCETYPEs::GFXAPI_MATINST;
		std::cout << "Path of Material Instance is: " << RESOURCE->PATH << std::endl;
		EDITOR_FILESYSTEM->Add_anAsset_toAssetList(RESOURCE);
		new Status_Window("Successfully created!");
	}
	
}