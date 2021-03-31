#include "Texture_Loader.h"
#include "Editor/FileSystem/EditorFileSystem_Core.h"
//To show import status
#include "Editor/Editors/Status_Window.h"
#include "Editor/Editors/Properties_Window.h"
//To send textures to GPU memory
#include "GFX/GFX_Core.h"
#include "Editor/FileSystem/ResourceTypes/ResourceTYPEs.h"

using namespace TuranAPI;

//To import textures from 3rd party data formats
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <string>

namespace TuranEditor {
	//Output Path should be Directory + Name like "C:/dev/Content/FirstTexture". Every Texture has .texturecont extension!
	Resource_Identifier* Texture_Loader::Import_Texture(const char* PATH, const bool& flip_vertically, const GFX_API::Texture_Properties& Properties) {
		GFX_API::TEXTURE_CHANNELs CHANNEL_TYPE = GFX_API::TEXTURE_CHANNELs::API_TEXTURE_RGB8UB;
		int WIDTH, HEIGHT, CHANNELs;
		stbi_set_flip_vertically_on_load(flip_vertically);
		unsigned char* data = stbi_load(PATH, &WIDTH, &HEIGHT, &CHANNELs, 0);
		if (data == nullptr) {
			LOG_ERROR("Texture isn't loaded from source! Data is nullptr!");
			return nullptr;
		}
		switch (CHANNELs) {
		case 0:
			LOG_ERROR("Texture has 0 channels, that's not possible! Loading failed!");
			return nullptr;
		case 1:
			CHANNEL_TYPE = GFX_API::TEXTURE_CHANNELs::API_TEXTURE_R8UB;
			break;
		case 2:
			CHANNEL_TYPE = GFX_API::TEXTURE_CHANNELs::API_TEXTURE_RA8UB;
			break;
		case 3:
			CHANNEL_TYPE = GFX_API::TEXTURE_CHANNELs::API_TEXTURE_RGB8UB;
			break;
		case 4:
			CHANNEL_TYPE = GFX_API::TEXTURE_CHANNELs::API_TEXTURE_RGBA8UB;
			break;
		default:
			LOG_ERROR("Texture has unsupported number of channels, that's not possible! Loading failed!");
			return nullptr;
		}
		//If application arrives here, loading is successful!
		GFX_API::Texture_Resource* texture_resource = new GFX_API::Texture_Resource;
		texture_resource->Properties.DIMENSION = Properties.DIMENSION;
		texture_resource->Properties.MIPMAP_FILTERING = Properties.MIPMAP_FILTERING;
		texture_resource->Properties.WRAPPING = Properties.WRAPPING;
		texture_resource->Properties.CHANNEL_TYPE = CHANNEL_TYPE;
		texture_resource->DATA = data;
		texture_resource->WIDTH = WIDTH;
		texture_resource->HEIGHT = HEIGHT;
		texture_resource->DATA_SIZE = texture_resource->WIDTH * texture_resource->HEIGHT * CHANNELs;
		texture_resource->OP_TYPE = GFX_API::BUFFER_VISIBILITY::CPUEXISTENCE_GPUREADONLY;
		texture_resource->Properties.TYPE = GFX_API::TEXTURE_TYPEs::COLOR_TEXTURE;

		if (texture_resource) {
			Resource_Identifier* RESOURCE = new Resource_Identifier;
			RESOURCE->PATH = PATH;
			RESOURCE->DATA = texture_resource;
			RESOURCE->TYPE = RESOURCETYPEs::GFXAPI_TEXTURE;

			EDITOR_FILESYSTEM->Add_anAsset_toAssetList(RESOURCE);
			//LOG_STATUS("Texture is loaded successfully!");
			return RESOURCE;
		}
		else {
			LOG_ERROR("Texture isn't loaded!");
			return nullptr;
		}

	}
}