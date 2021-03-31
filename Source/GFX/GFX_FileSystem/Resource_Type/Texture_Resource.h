#pragma once
#include "GFX/GFX_Includes.h"
#include "GFX/GFX_ENUMs.h"

enum class RESOURCETYPEs : char;

namespace GFX_API {
	struct GFXAPI Texture_Properties {
		TEXTURE_DIMENSIONs DIMENSION = TEXTURE_DIMENSIONs::TEXTURE_2D;
		TEXTURE_MIPMAPFILTER MIPMAP_FILTERING = TEXTURE_MIPMAPFILTER::API_TEXTURE_LINEAR_FROM_1MIP;
		TEXTURE_WRAPPING WRAPPING = TEXTURE_WRAPPING::API_TEXTURE_REPEAT;
		TEXTURE_CHANNELs CHANNEL_TYPE = TEXTURE_CHANNELs::API_TEXTURE_RGB8UB;
		TEXTURE_TYPEs TYPE = TEXTURE_TYPEs::COLOR_TEXTURE;
		Texture_Properties();
		Texture_Properties(TEXTURE_DIMENSIONs dimension, TEXTURE_MIPMAPFILTER mipmap_filtering = TEXTURE_MIPMAPFILTER::API_TEXTURE_LINEAR_FROM_1MIP,
			TEXTURE_WRAPPING wrapping = TEXTURE_WRAPPING::API_TEXTURE_REPEAT, TEXTURE_CHANNELs channel_type = TEXTURE_CHANNELs::API_TEXTURE_RGB8UB);
	};

	/*
		Texture Resource Specifications:
			1) You can use textures to just read on GPU (probably for object material rendering), read-write on GPU (using compute shader to write an image), render target (framebuffer attachment)
			2) Modern APIs let you use a texture in anyway, it's left for your care. But OpenGL doesn't let this situation, you are defining your texture's type in creation proccess
			3) GFX_API doesn't do any checks for type safety, so it's up to you! That means it's possible to access a read-only buffer as a write buffer in a shader
			4) If Has_Mipmaps is true, mipmap generation is triggered on the GPU (Want to support uploading mipmaps in future).
			5) If OP_TYPE is CPUEXISTENCE_xxx, you have to upload texture data in GFXContentManager->Create_Texture. You shouldn't call GFXContentManager->Upload_Texture()

		Texture Types:
			1) Sampler Texture
				a) This is a read only texture for GPU (Object material rendering etc). TEXTURE_TYPEs enum should be either COLOR_TEXTURE or OPACITY_TEXTURE
				b) OP_TYPE should be either CPUEXISTENCE_GPUREADONLY or CPUREADWRITE_GPUREADONLY
			2) Image Texture
				a) This is a read-write texture for GPU, but not a framebuffer attachment
				b) TEXTURE_TYPEs: COLOR_TEXTURE or OPACITY_TEXTURE
				c) OP_TYPEs: CPUEXISTENCE_GPUREADWRITE, CPUREADWRITE_GPUREADWRITE, CPUREAD_GPUREADWRITE
				d) TEXTURE_CHANNELs: ***** WRITE THESE
			3) Render Target Texture
				a) This texture is used as framebuffer attachment for hardware rasterization
				b) TEXTURE_TYPEs: COLORRT_TEXTURE, DEPTHTEXTURE, DEPTHSTENCIL
				c) OP_TYPEs: CPUEXISTENCE_GPUREADWRITE, CPUREADWRITE_GPUREADWRITE, CPUREAD_GPUREADWRITE
				d) TEXTURE_CHANNELs: ***** WRITE_THESE
	*/
	class GFXAPI Texture_Resource {
	public:
		Texture_Properties Properties;
		BUFFER_VISIBILITY OP_TYPE;
		unsigned int WIDTH, HEIGHT, DATA_SIZE;
		unsigned char* DATA;
		bool Has_Mipmaps;
		Texture_Resource();
		~Texture_Resource();
	};
}

