#include "GFX_ENUMs.h"

namespace GFX_API {
	GFXAPI unsigned int Get_UNIFORMTYPEs_SIZEinbytes(DATA_TYPE uniform) {
		switch (uniform)
		{
		case DATA_TYPE::VAR_FLOAT32:
		case DATA_TYPE::VAR_INT32:
		case DATA_TYPE::VAR_UINT32:
			return 4;
		case DATA_TYPE::VAR_BYTE8:
		case DATA_TYPE::VAR_UBYTE8:
			return 1;
		case DATA_TYPE::VAR_VEC2:
			return 8;
		case DATA_TYPE::VAR_VEC3:
			return 12;
		case DATA_TYPE::VAR_VEC4:
			return 16;
		case DATA_TYPE::VAR_MAT4x4:
			return 64;
		default:
			LOG_CRASHING("Uniform's size in bytes isn't set in GFX_ENUMs.cpp!");
			return 0;
		}
	}

	GFXAPI const char* Find_UNIFORM_VARTYPE_Name(DATA_TYPE uniform_var_type) {
		switch (uniform_var_type) {
		case DATA_TYPE::VAR_UINT32:
			return "Unsigned Integer 32-bit";
		case DATA_TYPE::VAR_INT32:
			return "Signed Integer 32-bit";
		case DATA_TYPE::VAR_FLOAT32:
			return "Float 32-bit";
		case DATA_TYPE::VAR_VEC2:
			return "Vec2 (2 float)";
		case DATA_TYPE::VAR_VEC3:
			return "Vec3 (3 float)";
		case DATA_TYPE::VAR_VEC4:
			return "Vec4 (4 float)";
		case DATA_TYPE::VAR_MAT4x4:
			return "Matrix 4x4";
		case DATA_TYPE::VAR_UBYTE8:
			return "Unsigned Byte 8-bit";
		case DATA_TYPE::VAR_BYTE8:
			return "Signed Byte 8-bit";
		default:
			return "Error, Uniform_Var_Type isn't supported by Find_UNIFORM_VARTYPE_Name!\n";
		}
	}


	
	GFXAPI const char* GetNameOf_TextureWRAPPING(TEXTURE_WRAPPING WRAPPING) {
		switch (WRAPPING) {
		case TEXTURE_WRAPPING::API_TEXTURE_REPEAT:
			return "Repeat";
		case TEXTURE_WRAPPING::API_TEXTURE_MIRRORED_REPEAT:
			return "Mirrored Repeat";
		case TEXTURE_WRAPPING::API_TEXTURE_CLAMP_TO_EDGE:
			return "Clamp to Edge";
		default:
			LOG_ERROR("GetNameOf_TextureWRAPPING doesn't support this wrapping type!");
			return "ERROR";
		}
	}
	GFXAPI vector<const char*> GetNames_TextureWRAPPING() {
		return vector<const char*>{
			GetNameOf_TextureWRAPPING(TEXTURE_WRAPPING::API_TEXTURE_REPEAT), GetNameOf_TextureWRAPPING(TEXTURE_WRAPPING::API_TEXTURE_MIRRORED_REPEAT),
				GetNameOf_TextureWRAPPING(TEXTURE_WRAPPING::API_TEXTURE_CLAMP_TO_EDGE)
		};
	}
	GFXAPI TEXTURE_WRAPPING GetTextureWRAPPING_byIndex(unsigned int Index) {
		switch (Index) {
		case 0:
			return TEXTURE_WRAPPING::API_TEXTURE_REPEAT;
		case 1:
			return TEXTURE_WRAPPING::API_TEXTURE_MIRRORED_REPEAT;
		case 2:
			return TEXTURE_WRAPPING::API_TEXTURE_CLAMP_TO_EDGE;
		default:
			LOG_ERROR("GetTextureWRAPPING_byIndex doesn't support this index!\n");
		}
	}


	GFXAPI const char* GetNameOf_TextureCHANNELs(TEXTURE_CHANNELs CHANNEL) {
		LOG_NOTCODED("Texture Channels enum has changed but function related to it hasn't. Fix it!", true);
		switch (CHANNEL) {
		case TEXTURE_CHANNELs::API_TEXTURE_RGB8UB:
			return "RGB8UB";
		default:
			LOG_ERROR("GetNameOf_TextureCHANNELs doesn't support this channel type!");
		}
	}
	GFXAPI vector<const char*> GetNames_TextureCHANNELs() {
		return vector<const char*>{

		};
	}
	GFXAPI TEXTURE_CHANNELs GetTextureCHANNEL_byIndex(unsigned int Index) {
		LOG_NOTCODED("Texture Channels enum has changed but function related to it hasn't. Fix it!", true);
		switch (Index) {
		default:
			LOG_ERROR("GetTextureCHANNEL_byIndex doesn't support this index!");
		}
	}
	GFXAPI unsigned int GetIndexOf_TextureCHANNEL(TEXTURE_CHANNELs CHANNEL) {
		LOG_NOTCODED("Texture Channels enum has changed but function related to it hasn't. Fix it!", true);
		switch (CHANNEL) {
		default:
			LOG_ERROR("GetIndexOf_TextureCHANNEL doesn't support this channel type!");
		}
	}



	GFXAPI const char* GetNameof_SHADERLANGUAGE(SHADER_LANGUAGEs LANGUAGE) {
		switch (LANGUAGE) {
		case SHADER_LANGUAGEs::GLSL:
			return "GLSL";
		case SHADER_LANGUAGEs::HLSL:
			return "HLSL";
		case SHADER_LANGUAGEs::SPIRV:
			return "SPIR-V";
		case SHADER_LANGUAGEs::TSL:
			return "TSL";
		default:
			LOG_ERROR("GetNameof_SHADERLANGUAGE() doesn't support this language!");
		}
	}
	GFXAPI vector<const char*> GetNames_SHADERLANGUAGEs() {
		return vector<const char*> {
				GetNameof_SHADERLANGUAGE(SHADER_LANGUAGEs::GLSL), GetNameof_SHADERLANGUAGE(SHADER_LANGUAGEs::HLSL), 
				GetNameof_SHADERLANGUAGE(SHADER_LANGUAGEs::SPIRV), GetNameof_SHADERLANGUAGE(SHADER_LANGUAGEs::TSL)
		};
	}
	GFXAPI SHADER_LANGUAGEs GetSHADERLANGUAGE_byIndex(unsigned int Index) {
		switch (Index) {
		case 0:
			return SHADER_LANGUAGEs::GLSL;
		case 1:
			return SHADER_LANGUAGEs::HLSL;
		case 2:
			return SHADER_LANGUAGEs::SPIRV;
		case 3:
			return SHADER_LANGUAGEs::TSL;
		default:
			LOG_ERROR("GetSHADERLANGUAGE_byIndex() doesn't support this index!");
		}
	}




	GFXAPI const char* GetNameof_SHADERSTAGE(SHADER_STAGE SHADERSTAGE) {
		switch (SHADERSTAGE) {
		case SHADER_STAGE::VERTEXSTAGE:
			return "Vertex Shader";
		case SHADER_STAGE::FRAGMENTSTAGE:
			return "Fragment Shader";
		default:
			LOG_ERROR("GetNameof_SHADERSTAGE() doesn't support this language!");
		}
	}
	GFXAPI vector<const char*> GetNames_SHADERSTAGEs() {
		return vector<const char*> {
			GetNameof_SHADERSTAGE(SHADER_STAGE::VERTEXSTAGE) , GetNameof_SHADERSTAGE(SHADER_STAGE::FRAGMENTSTAGE)
		};
	}
	GFXAPI SHADER_STAGE GetSHADERSTAGE_byIndex(unsigned int Index) {
		switch (Index) {
		case 0:
			return SHADER_STAGE::VERTEXSTAGE;
		case 1:
			return SHADER_STAGE::FRAGMENTSTAGE;
		default:
			LOG_ERROR("GetSHADERSTAGE_byIndex() doesn't support this index!");
		}
	}


	unsigned int GetKeyIndex(KEYBOARD_KEYs key) {
		switch (key)
		{
		case GFX_API::KEYBOARD_KEYs::KEYBOARD_W:
			return 0;
		case GFX_API::KEYBOARD_KEYs::KEYBOARD_A:
			return 1;
		case GFX_API::KEYBOARD_KEYs::KEYBOARD_S:
			return 2;
		case GFX_API::KEYBOARD_KEYs::KEYBOARD_D:
			return 3;
		case GFX_API::KEYBOARD_KEYs::KEYBOARD_C:
			return 4;
		case GFX_API::KEYBOARD_KEYs::KEYBOARD_NP_2:
			return 5;
		case GFX_API::KEYBOARD_KEYs::KEYBOARD_NP_4:
			return 6;
		case GFX_API::KEYBOARD_KEYs::KEYBOARD_NP_6:
			return 7;
		case GFX_API::KEYBOARD_KEYs::KEYBOARD_NP_8:
			return 8;
		default:
			LOG_CRASHING("GetKeyIndex() doesn't support this key!");
			return 0;
		}
	}
	unsigned int GetMouseButtonIndex(MOUSE_BUTTONs button) {
		switch (button)
		{
		case GFX_API::MOUSE_BUTTONs::MOUSE_LEFT_CLICK:
			return 0;
		case GFX_API::MOUSE_BUTTONs::MOUSE_WHEEL_CLICK:
			return 1;
		case GFX_API::MOUSE_BUTTONs::MOUSE_RIGHT_CLICK:
			return 2;
		default:
			LOG_CRASHING("GetMouseButtonIndex() doesn't support this key!");
			return 0;
		}
	}
}