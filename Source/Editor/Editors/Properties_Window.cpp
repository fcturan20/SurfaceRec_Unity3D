#include "Properties_Window.h"
#include "Editor/FileSystem/ResourceTypes/ResourceTYPEs.h"
#include "GFX/GFX_FileSystem/Resource_Type/Texture_Resource.h"
#include "Editor/FileSystem/ResourceTypes/Model_Resource.h"
#include "GFX/GFX_Core.h"
#include "Editor/FileSystem/EditorFileSystem_Core.h"

#include "Editor/RenderContext/Game_RenderGraph.h"
#include "Editor/RenderContext/Draw Passes/Draw_Passes.h"

#include "Status_Window.h"
#include "Scene_Editor.h"

#include <string>

using namespace TuranAPI;

namespace TuranEditor {
	ResourceProperties_Window::ResourceProperties_Window(Resource_Identifier* resource) : IMGUI_WINDOW(resource->Get_Name().c_str()) {
		RESOURCE = resource;
		IMGUI_REGISTERWINDOW(this);

		if (RESOURCE->TYPE == RESOURCETYPEs::EDITOR_STATICMODEL) {
			RenderGraph = new Game_RenderGraph;
			Static_Model* MODEL = (Static_Model*)RESOURCE->DATA;
			vector<unsigned int> MESHIDs = MODEL->Upload_toGPU();
			for (unsigned int i = 0; i < MESHIDs.size(); i++) {
				GFX_API::DrawCall Call;
				Call.JoinedDrawPasses = Main_DrawPass::Get_BitMaskFlag();
				Call.MeshBuffer_ID = MESHIDs[i];
				Call.ShaderInstance_ID = 0;
				RenderGraph->Register_DrawCall(Call);
			}
		}
		else if (RESOURCE->TYPE == RESOURCETYPEs::GFXAPI_TEXTURE) {
			GFX_API::Texture_Resource* TEXTURE = (GFX_API::Texture_Resource*)RESOURCE->DATA;
			if (!GFXContentManager->Find_GFXTexture_byID(RESOURCE->ID)) {
				GFXContentManager->Create_Texture(TEXTURE, RESOURCE->ID);
			}
		}
	}

	void ResourceProperties_Window::Run_Window() {
		if (!Is_Window_Open) {
			IMGUI_DELETEWINDOW(this);
			return;
		}
		if (!IMGUI->Create_Window(Window_Name.c_str(), Is_Window_Open, false)) {
			IMGUI->End_Window();
			return;
		}
		switch (RESOURCE->TYPE) {
		case RESOURCETYPEs::EDITOR_STATICMODEL:
			Show_Model_Properties();
			break;
		case RESOURCETYPEs::GFXAPI_TEXTURE:
			Show_Texture_Properties();
			break;
		case RESOURCETYPEs::GFXAPI_MATTYPE:
			Show_MaterialType_Properties();
			break;
		case RESOURCETYPEs::GFXAPI_MATINST:
			Show_MaterialInstance_Properties();
			break;
		case RESOURCETYPEs::EDITOR_SCENE:
			new Scene_Editor(RESOURCE);
			Is_Window_Open = false;
			IMGUI_DELETEWINDOW(this);
			break;
		case RESOURCETYPEs::GFXAPI_SHADERSOURCE:
			Show_ShaderSource_Properties();
			break;
		default:
			new Status_Window("This type's properties can't be shown by Properties Window!");
			Is_Window_Open = false;
		}
		IMGUI->End_Window();
	}

	void ResourceProperties_Window::Show_ShaderSource_Properties() {
		GFX_API::ShaderSource_Resource* SHADERSOURCE = (GFX_API::ShaderSource_Resource*)RESOURCE->DATA;
		IMGUI->Text(("ID: " + to_string(RESOURCE->ID)).c_str());
		IMGUI->Text(("Shader Stage: " + string(GFX_API::GetNameof_SHADERSTAGE(SHADERSOURCE->STAGE))).c_str());
		IMGUI->Text(("Shader Language: " + string(GFX_API::GetNameof_SHADERLANGUAGE(SHADERSOURCE->LANGUAGE))).c_str());
		IMGUI->Text("Code:");
		IMGUI->Separator();
		IMGUI->Text(SHADERSOURCE->SOURCE_CODE.c_str());
	}

	void ResourceProperties_Window::Show_MaterialType_Properties() {
		GFX_API::Material_Type* material_type_resource = (GFX_API::Material_Type*)RESOURCE->DATA;
		string MatTypeName = "Material Type Name: ";
		MatTypeName.append(RESOURCE->Get_Name());
		IMGUI->Text(MatTypeName.c_str());

		GFX_API::Material_Uniform* UNIFORM = nullptr;
		if (IMGUI->Begin_Tree("Uniform List")) {
			for (unsigned int i = 0; i < material_type_resource->UNIFORMs.size(); i++) {
				UNIFORM = &material_type_resource->UNIFORMs[i];

				if (IMGUI->Begin_Tree("number->string isn't supported yet!")) {
					string UniformName = "Uniform Name: ";
					UniformName.append(UNIFORM->VARIABLE_NAME);
					IMGUI->Text(UniformName.c_str());
					IMGUI->Text(("Uniform Variable Type: " + std::string(Find_UNIFORM_VARTYPE_Name(UNIFORM->VARIABLE_TYPE))).c_str());
					IMGUI->End_Tree();
				}
			}
			IMGUI->End_Tree();
		}
	}

	void ResourceProperties_Window::Show_MaterialInstance_Properties() {
		GFX_API::Material_Instance* material_instance_resource = (GFX_API::Material_Instance*)RESOURCE->DATA;
		string MatInstName = "Material Instance Name: ";
		MatInstName.append(RESOURCE->Get_Name());
		IMGUI->Text(MatInstName.c_str());
		string MatTypeName = "Material Type Name: ";
		MatTypeName.append(
			EDITOR_FILESYSTEM->Find_ResourceIdentifier_byID(material_instance_resource->Material_Type)->Get_Name()
		);
		IMGUI->Text(MatTypeName.c_str());

		GFX_API::Material_Uniform* UNIFORM = nullptr;
		if (IMGUI->Begin_Tree("Uniform List")) {
			for (unsigned int i = 0; i < material_instance_resource->UNIFORM_LIST.size(); i++) {
				UNIFORM = &material_instance_resource->UNIFORM_LIST[i];

				if (IMGUI->Begin_Tree("number->string isn't supported yet!")) {
					string UniformName = "Uniform Name: ";
					UniformName.append(UNIFORM->VARIABLE_NAME);
					IMGUI->Text(UniformName.c_str());
					IMGUI->Text(("Uniform Variable Type: " + std::string(Find_UNIFORM_VARTYPE_Name(UNIFORM->VARIABLE_TYPE))).c_str());
					IMGUI->End_Tree();
				}
			}
			IMGUI->End_Tree();
		}
	}


	void ResourceProperties_Window::Show_Texture_Properties() {
		GFX_API::Texture_Resource* TEXTURE = (GFX_API::Texture_Resource*)RESOURCE->DATA;
		IMGUI->Text(("ID: " + to_string(RESOURCE->ID)).c_str());
		selected_texturechannelindex = GFX_API::GetIndexOf_TextureCHANNEL(TEXTURE->Properties.CHANNEL_TYPE);
		if (IMGUI->SelectList_OneLine("Texture Channels", &selected_texturechannelindex, GFX_API::GetNames_TextureCHANNELs())) {
			TEXTURE->Properties.CHANNEL_TYPE = GFX_API::GetTextureCHANNEL_byIndex(selected_texturechannelindex);
			GFXContentManager->Unload_Texture(RESOURCE->ID);
			GFXContentManager->Create_Texture(TEXTURE, RESOURCE->ID);
		}
		IMGUI->Display_Texture(RESOURCE->ID, TEXTURE->WIDTH, TEXTURE->HEIGHT);
	}

	void ResourceProperties_Window::Show_Model_Properties() {
		Static_Model* model_data_resource = (Static_Model*)RESOURCE->DATA;
		string ModelName = "Model Name: ";
		ModelName.append(RESOURCE->Get_Name());
		IMGUI->Text(ModelName.c_str());
		IMGUI->Text(("ID: " + to_string(RESOURCE->ID)).c_str());
		string Mesh_Number = "Mesh Number: ";
		Mesh_Number.append(std::to_string(model_data_resource->MESHes.size()).c_str());
		IMGUI->Text(Mesh_Number.c_str());
		unsigned int data_size = 0;
		for (unsigned int i = 0; i < model_data_resource->MESHes.size(); i++) {
			data_size += model_data_resource->MESHes[i]->VERTEXDATA_SIZE;
		}
		IMGUI->Text(("Data Size: " + to_string(data_size)).c_str());
		for (unsigned int i = 0; i < model_data_resource->MESHes.size(); i++) {
			if (IMGUI->Begin_Tree(("Mesh " + to_string(i)).c_str())) {
				Static_Mesh_Data* MESH = model_data_resource->MESHes[i];
				IMGUI->Text(("Material Index: " + to_string(MESH->Material_Index)).c_str());
				const GFX_API::VertexAttributeLayout& Layout = MESH->DataLayout;
				IMGUI->Text(("Data  Size per Vertex: " + to_string(Layout.size_pervertex)).c_str());
				IMGUI->Text(("Vertex Number: " + to_string(MESH->VERTEX_NUMBER)).c_str());
				for (unsigned int j = 0; j < Layout.Attributes.size(); j++) {
					if (IMGUI->Begin_Tree(("Attribute " + to_string(j)).c_str())) {
						const GFX_API::VertexAttribute& Attribute = Layout.Attributes[j];
						IMGUI->Text(("Attribute Name: " + Attribute.AttributeName).c_str());
						IMGUI->Text(("Attribute Index: " + to_string(Attribute.Index)).c_str());
						IMGUI->Text(("Attribute Start Offset: " + to_string(Attribute.Start_Offset)).c_str());
						IMGUI->Text(("Attribute Stride: " + to_string(Attribute.Stride)).c_str());
						IMGUI->Text(("Attribute Data Type: " + string(GFX_API::Find_UNIFORM_VARTYPE_Name(Attribute.DATATYPE))).c_str());
						IMGUI->End_Tree();
					}
				}
				IMGUI->End_Tree();
			}
		}
	}





}