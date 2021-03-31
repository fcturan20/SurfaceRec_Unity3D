#include "FirstCompute.h"
#include "Editor/FileSystem/EditorFileSystem_Core.h"
#include "Editor/RenderContext/Editor_DataManager.h"
#include "Editor/FileSystem/ResourceTypes/Resource_Identifier.h"
#include "Editor/FileSystem/ResourceTypes/ResourceTYPEs.h"
#define TEXTURE_WIDTH 360
#define TEXTURE_HEIGHT 360
#define RAYTRACECAMERABUFFER_SIZE 80

using namespace GFX_API;

namespace TuranEditor {

	unsigned int FirstCompute::OUTPUTTEXTURE_ID = 0, FirstCompute::CS_ID = 0, FirstCompute::CSinst_ID = 0, FirstCompute::RAYTRACECAMERABUFFER_ID = 0;
	void* FirstCompute::RAYTRACECAMERABUFFER_DATA = new bool[RAYTRACECAMERABUFFER_SIZE]{};
	vec3 FirstCompute::RAYTRACECAMERA_POS(0, 0, 0), FirstCompute::RAYTRACECAMERA_FRONTVECTOR(0, 0, 1);
	FirstCompute::FirstCompute() {
		//Ray Trace Camera Buffer
		{
			RAYTRACECAMERABUFFER_ID = GFXContentManager->Create_GlobalBuffer("RAYTRACE_CAMERA", RAYTRACECAMERABUFFER_DATA, RAYTRACECAMERABUFFER_SIZE, GFX_API::BUFFER_VISIBILITY::CPUREADWRITE_GPUREADWRITE);
		}

		//COMPUTE SHADER LOAD
		GFX_API::ComputeShader_Resource* First_CS = new ComputeShader_Resource;
		First_CS->LANGUAGE = SHADER_LANGUAGEs::GLSL;
		First_CS->SOURCE_CODE = *TAPIFILESYSTEM::Read_TextFile("C:/dev/Test Renderer/Content/FirstComputeShader.comp");
		//Global Buffer Binding
		{
			GlobalBuffer_Access RAYTRACECAMERA_Access;
			RAYTRACECAMERA_Access.ACCESS_TYPE = GFX_API::OPERATION_TYPE::READ_ONLY;
			RAYTRACECAMERA_Access.BUFFER_ID = RAYTRACECAMERABUFFER_ID;
			First_CS->GLOBALBUFFERs.push_back(RAYTRACECAMERA_Access);
			GlobalBuffer_Access LIGHTs_Access;
			LIGHTs_Access.ACCESS_TYPE = GFX_API::OPERATION_TYPE::READ_ONLY;
			LIGHTs_Access.BUFFER_ID = Editor_RendererDataManager::LIGHTs_GLOBALBUFFERID;
			First_CS->GLOBALBUFFERs.push_back(LIGHTs_Access);
		}

		//Output Image
		{
			Texture_Access OUTPUT;
			OUTPUT.DIMENSIONs = TEXTURE_DIMENSIONs::TEXTURE_2D;
			OUTPUT.CHANNELs = TEXTURE_CHANNELs::API_TEXTURE_RGBA32F;
			OUTPUT.OP_TYPE = OPERATION_TYPE::WRITE_ONLY;
			OUTPUT.ACCESS_TYPE = TEXTURE_ACCESS::IMAGE_OPERATION;
			OUTPUT.BINDING_POINT = 0;
			OUTPUT.TEXTURE_ID = 0;
			First_CS->TEXTUREs.push_back(OUTPUT);
		}
		//Compute Shader Resource Creation
		{
			Resource_Identifier* RESOURCE = new Resource_Identifier;
			RESOURCE->TYPE = RESOURCETYPEs::GFXAPI_COMPUTESHADER;
			RESOURCE->DATA = First_CS;
			EDITOR_FILESYSTEM->Add_anAsset_toAssetList(RESOURCE);
			string compilation_status;		//I don't care, because it will be compiled anyway!
			//Add_anAsset_toAssetList gave an ID to the Material Type, link it now!
			GFXContentManager->Compile_ComputeShader(First_CS, RESOURCE->ID, &compilation_status);
			CS_ID = RESOURCE->ID;
		}

		//OUTPUT IMAGE CREATION
		{
			GFX_API::Texture_Resource* TEXTURE = new GFX_API::Texture_Resource;
			TEXTURE->DATA = nullptr; TEXTURE->DATA_SIZE = TEXTURE_WIDTH * TEXTURE_HEIGHT * 16;
			TEXTURE->Has_Mipmaps = false; TEXTURE->WIDTH = TEXTURE_WIDTH; TEXTURE->HEIGHT = TEXTURE_HEIGHT;
			TEXTURE->Properties.CHANNEL_TYPE = TEXTURE_CHANNELs::API_TEXTURE_RGBA32F;
			TEXTURE->Properties.TYPE = TEXTURE_TYPEs::COLOR_TEXTURE;
			TEXTURE->OP_TYPE = BUFFER_VISIBILITY::CPUEXISTENCE_GPUREADWRITE;

			Resource_Identifier* RESOURCE = new Resource_Identifier;
			RESOURCE->TYPE = RESOURCETYPEs::GFXAPI_TEXTURE;
			RESOURCE->DATA = RESOURCE;
			EDITOR_FILESYSTEM->Add_anAsset_toAssetList(RESOURCE);
			string compilation_status;		//I don't care, because it will be compiled anyway!
			OUTPUTTEXTURE_ID = RESOURCE->ID;

			GFXContentManager->Create_Texture(TEXTURE, RESOURCE->ID);
		}


		//COMPUTE SHADER INSTANCE LOAD
		GFX_API::ComputeShader_Instance* First_CSinst = new ComputeShader_Instance;
		First_CSinst->ComputeShader = CS_ID;
		First_CSinst->TEXTURE_LIST = First_CS->TEXTUREs;
		First_CSinst->UNIFORM_LIST = First_CS->UNIFORMs;
		for (unsigned int i = 0; i < First_CSinst->TEXTURE_LIST.size(); i++) {
			Texture_Access& TEXTURE = First_CSinst->TEXTURE_LIST[i];
			if (TEXTURE.BINDING_POINT == 0 && TEXTURE.ACCESS_TYPE == TEXTURE_ACCESS::IMAGE_OPERATION) {
				TEXTURE.TEXTURE_ID = OUTPUTTEXTURE_ID;
			}
		}
		//Resource Compilation
		{
			Resource_Identifier* RESOURCE = new Resource_Identifier;
			RESOURCE->TYPE = RESOURCETYPEs::GFXAPI_COMPUTESHADERINST;
			RESOURCE->DATA = First_CSinst;
			EDITOR_FILESYSTEM->Add_anAsset_toAssetList(RESOURCE);
			string compilation_status;		//I don't care, because it will be compiled anyway!
			CSinst_ID = RESOURCE->ID;
			ComputeShaders.push_back(First_CSinst);
		}
	}

	void FirstCompute::Execute() {
		//Update Ray Trace Camera Buffer
		{
			memcpy(RAYTRACECAMERABUFFER_DATA, &RAYTRACECAMERA_POS, 12);
			mat4 CAMERA_to_WORLD;
			CAMERA_to_WORLD = inverse(lookAt(RAYTRACECAMERA_POS, RAYTRACECAMERA_POS + RAYTRACECAMERA_FRONTVECTOR, vec3(0, 1, 0)));
			memcpy(((bool*)RAYTRACECAMERABUFFER_DATA) + 16, &CAMERA_to_WORLD, 64);
			GFXContentManager->Upload_GlobalBuffer(RAYTRACECAMERABUFFER_ID);
		}
		GFXRENDERER->Compute_Dispatch(ComputeShaders[0], vec3(TEXTURE_WIDTH, TEXTURE_HEIGHT, 1));
	}
}