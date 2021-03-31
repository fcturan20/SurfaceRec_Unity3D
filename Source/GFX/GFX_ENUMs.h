#pragma once
#include "GFX_Includes.h"

//This header is used to create a GL data type classification for all of the TuranEngine
//For example; Texture types, Mipmap settings, Desktop modes etc.
//In GL specific [GFX_API]_ENUMs.h, these data types have to be converted to GL specific data types
//For example; GFX_TEXTURE_2D data type here, should be converted to GL_TEXTURE_2D in OGL3_ENUMs.h
namespace GFX_API {
	//Variable Types!
	enum class DATA_TYPE : unsigned char {
		VAR_UBYTE8 = 0, VAR_BYTE8, VAR_UINT32, VAR_INT32, VAR_FLOAT32,
		VAR_VEC2, VAR_VEC3, VAR_VEC4, VAR_MAT4x4
	};
	GFXAPI unsigned int Get_UNIFORMTYPEs_SIZEinbytes(DATA_TYPE uniform);
	GFXAPI const char* Find_UNIFORM_VARTYPE_Name(DATA_TYPE uniform_var_type);

	enum class RT_ATTACHMENTs : unsigned char {
		TEXTURE_ATTACHMENT_COLOR0,
		TEXTURE_ATTACHMENT_DEPTH,
		TEXTURE_ATTACHMENT_DEPTHSTENCIL
	};

	enum class OPERATION_TYPE : unsigned char {
		READ_ONLY,
		WRITE_ONLY,
		READ_AND_WRITE
	};

	enum class RT_READSTATE : unsigned char {
		CLEAR,
		DONT_CARE,
		LAST_FRAME
	};

	enum class DEPTH_TESTs : unsigned char {
		DEPTH_TEST_ALWAYS,
		DEPTH_TEST_NEVER,
		DEPTH_TEST_LESS,
		DEPTH_TEST_LEQUAL,
		DEPTH_TEST_GREATER,
		DEPTH_TEST_GEQUAL
	};

	enum class DEPTH_MODEs : unsigned char {
		DEPTH_READ_WRITE,
		DEPTH_READ_ONLY,
		DEPTH_OFF
	};

	enum class CULL_MODE : unsigned char {
		CULL_OFF,
		CULL_BACK,
		CULL_FRONT
	};

	enum class RENDERNODE_TYPEs {
		RENDERNODE_DRAWPASS,
		RENDERNODE_COMPUTE,
		RENDERNODE_COPYPASS
	};

	enum class TEXTURE_DIMENSIONs : unsigned char {
		TEXTURE_2D = 0,
		TEXTURE_3D = 1
	};

	enum class TEXTURE_MIPMAPFILTER : unsigned char {
		API_TEXTURE_NEAREST_FROM_1MIP,
		API_TEXTURE_LINEAR_FROM_1MIP,
		API_TEXTURE_NEAREST_FROM_2MIP,
		API_TEXTURE_LINEAR_FROM_2MIP
	};

	enum class TEXTURE_WRAPPING : unsigned char {
		API_TEXTURE_REPEAT,
		API_TEXTURE_MIRRORED_REPEAT,
		API_TEXTURE_CLAMP_TO_EDGE
	};
	GFXAPI const char* GetNameOf_TextureWRAPPING(TEXTURE_WRAPPING WRAPPING);
	GFXAPI vector<const char*> GetNames_TextureWRAPPING();
	GFXAPI TEXTURE_WRAPPING GetTextureWRAPPING_byIndex(unsigned int Index);

	enum class TEXTURE_CHANNELs : unsigned char {
		API_TEXTURE_RGBA32F,
		API_TEXTURE_RGBA32UI,
		API_TEXTURE_RGBA32I,
		API_TEXTURE_RGBA8UB,
		API_TEXTURE_RGBA8B,

		API_TEXTURE_RGB32F,
		API_TEXTURE_RGB32UI,
		API_TEXTURE_RGB32I,
		API_TEXTURE_RGB8UB,
		API_TEXTURE_RGB8B,

		API_TEXTURE_RA32F,
		API_TEXTURE_RA32UI,
		API_TEXTURE_RA32I,
		API_TEXTURE_RA8UB,
		API_TEXTURE_RA8B,

		API_TEXTURE_R32F,
		API_TEXTURE_R32UI,
		API_TEXTURE_R32I,
		API_TEXTURE_R8UB,
		API_TEXTURE_R8B
	};
	GFXAPI const char* GetNameOf_TextureCHANNELs(TEXTURE_CHANNELs CHANNEL);
	GFXAPI vector<const char*> GetNames_TextureCHANNELs();
	GFXAPI TEXTURE_CHANNELs GetTextureCHANNEL_byIndex(unsigned int Index);
	GFXAPI unsigned int GetIndexOf_TextureCHANNEL(TEXTURE_CHANNELs CHANNEL);

	enum class TEXTURE_TYPEs: unsigned char {
		COLOR_TEXTURE,
		OPACITYTEXTURE,
		COLORRT_TEXTURE,
		DEPTHTEXTURE,
		DEPTHSTENCIL
	};

	enum class TEXTURE_ACCESS : unsigned char {
		SAMPLER_OPERATION,
		IMAGE_OPERATION,
		FRAMEBUFFER_ATTACHMENT
	};


	enum class GPU_TYPEs : unsigned char{
		DISCRETE_GPU,
		INTEGRATED_GPU
	};

	enum class V_SYNC : unsigned char {
		VSYNC_OFF,
		VSYNC_DOUBLEBUFFER,
		VSYNC_TRIPLEBUFFER
	};

	enum class WINDOW_MODE : unsigned char {
		FULLSCREEN,
		WINDOWED
	};

	enum class GFX_APIs : unsigned char{
		OPENGL4 = 0,
		VULKAN = 1
	};

	enum class SHADER_LANGUAGEs : unsigned char {
		GLSL	= 0,
		HLSL	= 1,
		SPIRV	= 2,
		TSL		= 3
	};
	GFXAPI const char* GetNameof_SHADERLANGUAGE(SHADER_LANGUAGEs LANGUAGE);
	GFXAPI vector<const char*> GetNames_SHADERLANGUAGEs();
	GFXAPI SHADER_LANGUAGEs GetSHADERLANGUAGE_byIndex(unsigned int Index);

	enum class SHADER_STAGE : unsigned char {
		VERTEXSTAGE		= 0,
		//There should be TESSELATION etc.
		FRAGMENTSTAGE	= 1
	};
	GFXAPI const char* GetNameof_SHADERSTAGE(SHADER_STAGE SHADERSTAGE);
	GFXAPI vector<const char*> GetNames_SHADERSTAGEs();
	GFXAPI SHADER_STAGE GetSHADERSTAGE_byIndex(unsigned int Index);


	//If you change this enum, don't forget that Textures and Global Buffers uses this enum. So, consider them.

	enum class BUFFER_VISIBILITY : unsigned char {
		CPUREADWRITE_GPUREADONLY = 0,			//Use this when all the data responsibility on the CPU and GPU just reads it (Global Camera Matrixes, CPU Software Rasterization Depth reads from the GPU etc.)
		CPUREADWRITE_GPUREADWRITE,				//Use this when both CPU and GPU changes the data, no matter frequency. This has the worst performance on modern APIs
		CPUREADONLY_GPUREADWRITE,				//Use this when CPU needs feedback of the data proccessed on the GPU (Occlusion Culling on the CPU according to last frame's depth buffer etc.)
		CPUEXISTENCE_GPUREADWRITE,				//Use this for CPU never touchs the data and GPU has the all responsibility (Framebuffer attachments, GPU-driven pipeline buffers etc.)
		CPUEXISTENCE_GPUREADONLY,				//Use this when data never changes, just uploaded or deleted from the GPU (Object material rendering textures, constant global buffers etc.)
	};

	enum class KEYBOARD_KEYs : unsigned char {
		KEYBOARD_W = 0,
		KEYBOARD_A,
		KEYBOARD_S,
		KEYBOARD_D,
		KEYBOARD_C,
		KEYBOARD_NP_2,
		KEYBOARD_NP_4,
		KEYBOARD_NP_6,
		KEYBOARD_NP_8
	};

	enum class MOUSE_BUTTONs : unsigned char {
		MOUSE_LEFT_CLICK = 0,
		MOUSE_WHEEL_CLICK = 1,
		MOUSE_RIGHT_CLICK = 2,
	};

	struct InputHandler {
		bool isKeyPressed[9]{ false }, isMouseClicked[3]{false};
	};
	unsigned int GetKeyIndex(KEYBOARD_KEYs key);
	unsigned int GetMouseButtonIndex(MOUSE_BUTTONs button);
}