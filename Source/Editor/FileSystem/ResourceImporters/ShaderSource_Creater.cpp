#include "ShaderSource_Creater.h"
#include "Editor/FileSystem/ResourceTypes/ResourceTYPEs.h"
#include "Editor/Editors/Status_Window.h"
#include "GFX/GFX_Core.h"

using namespace GFX_API;
namespace TuranEditor {

	ShaderSource_CreationWindow::ShaderSource_CreationWindow() : IMGUI_WINDOW("ShaderSource Creation") {
		IMGUI_REGISTERWINDOW(this);
	}

	void ShaderSource_CreationWindow::Run_Window() {
		if (!Is_Window_Open) {
			IMGUI_DELETEWINDOW(this);
			return;
		}
		if (!IMGUI->Create_Window(Window_Name.c_str(), Is_Window_Open, false)) {
			IMGUI->End_Window();
		}
		if (IMGUI->Button("Create")) {
			SHADERSTAGE = GFX_API::GetSHADERSTAGE_byIndex(selected_shaderstageindex);
			LANGUAGE = GFX_API::GetSHADERLANGUAGE_byIndex(selected_shaderlanguageindex);
			
			ShaderSource_Resource* SHADERSOURCE = new ShaderSource_Resource;
			SHADERSOURCE->LANGUAGE = LANGUAGE;
			SHADERSOURCE->STAGE = SHADERSTAGE;
			SHADERSOURCE->SOURCE_CODE = CODE;

			Resource_Identifier* RESOURCE = new Resource_Identifier;
			RESOURCE->PATH = OUTPUT_FOLDER + OUTPUT_NAME + ".shadersource";
			RESOURCE->DATA = SHADERSOURCE;
			RESOURCE->TYPE = RESOURCETYPEs::GFXAPI_SHADERSOURCE;
			
			EDITOR_FILESYSTEM->Add_anAsset_toAssetList(RESOURCE);

			string compilation_status;
			GFXContentManager->Compile_ShaderSource(SHADERSOURCE, RESOURCE->ID, &compilation_status);
			new Status_Window(compilation_status.c_str());
			if (compilation_status == "Succesfully compiled!") {

			}
			else {
				GFXContentManager->Delete_ShaderSource(RESOURCE->ID);
				EDITOR_FILESYSTEM->Delete_anAsset_fromAssetList(RESOURCE->ID);
			}


		}
		IMGUI->Input_Text("Output Name", &OUTPUT_NAME);
		IMGUI->Input_Text("Output Folder", &OUTPUT_FOLDER);
		IMGUI->SelectList_OneLine("Shader Language", &selected_shaderlanguageindex, GFX_API::GetNames_SHADERLANGUAGEs());
		IMGUI->SelectList_OneLine("Shader Stage", &selected_shaderstageindex, GFX_API::GetNames_SHADERSTAGEs());
        if (!IMGUI->Begin_TabBar())
        {
            LOG_ERROR("IMGUI->Begin_TabBar() failed!");
            IMGUI->End_TabBar();
            IMGUI->End_Window();
            return;
        }
        if (IMGUI->Begin_TabItem("Code by Hand")) {
			IMGUI->Input_Paragraph_Text("Source Code", &CODE);
            IMGUI->End_TabItem();
        }
        if (IMGUI->Begin_TabItem("Load from text file")) {
			IMGUI->Input_Text("Source File", &SOURCECODE_PATH);
			if (IMGUI->Button("Read File")) {
				CODE = *TAPIFILESYSTEM::Read_TextFile(SOURCECODE_PATH.c_str());
			}
            IMGUI->Text(CODE.c_str());
            IMGUI->End_TabItem();
        }
        IMGUI->End_TabBar();



		IMGUI->End_Window();
	}
}