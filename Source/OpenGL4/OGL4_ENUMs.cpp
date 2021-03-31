#include "OGL4_ENUMs.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

using namespace TuranAPI;
using namespace GFX_API;

namespace OpenGL4 {
	unsigned int OGL4_API Find_RTTexture_FormatChannel(GFX_API::TEXTURE_CHANNELs channel_type);
	unsigned int OGL4_API Find_SamplerTexture_Format(TEXTURE_CHANNELs channels);

	unsigned int Find_Texture_Dimension(TEXTURE_DIMENSIONs dimension) {
		switch (dimension) {
		case TEXTURE_DIMENSIONs::TEXTURE_2D:
			return GL_TEXTURE_2D;
		default:
			LOG_CRASHING("Intended Texture Dimension isn't supported by OpenGL4 for now!");
			return NULL;
		}
	}


	unsigned int OGL4_API Find_glTexImage2D_InternalFormat(GFX_API::Texture_Resource* TEXTURE) {
		//Image or Render Target Texture
		if (TEXTURE->OP_TYPE == BUFFER_VISIBILITY::CPUEXISTENCE_GPUREADWRITE) {
			switch (TEXTURE->Properties.TYPE) {
			case TEXTURE_TYPEs::COLORRT_TEXTURE:
			case TEXTURE_TYPEs::DEPTHTEXTURE:
			case TEXTURE_TYPEs::DEPTHSTENCIL:
				return Find_RTTexture_FormatChannel(TEXTURE->Properties.CHANNEL_TYPE);
			case TEXTURE_TYPEs::COLOR_TEXTURE:
			case TEXTURE_TYPEs::OPACITYTEXTURE:
				return Find_ImageTexture_InternalFormat(TEXTURE->Properties.CHANNEL_TYPE);
			default:
				LOG_ERROR("Find_glTexImage2D_InternalFormat doesn't support this Texture_Type!");
			}
		}
		//Sampler Texture
		else if (TEXTURE->OP_TYPE == BUFFER_VISIBILITY::CPUEXISTENCE_GPUREADONLY) {
			return Find_SamplerTexture_Format(TEXTURE->Properties.CHANNEL_TYPE);
		}
		LOG_ERROR("Find_glTexImage2D_InternalFormat doesn't support this BUFFER_VISIBILITY!");
	}
	unsigned int OGL4_API Find_glTexImage2D_Format(GFX_API::Texture_Resource* TEXTURE) {
		//Image or Render Target Texture
		if (TEXTURE->OP_TYPE == BUFFER_VISIBILITY::CPUEXISTENCE_GPUREADWRITE) {
			switch (TEXTURE->Properties.TYPE) {
			case TEXTURE_TYPEs::COLORRT_TEXTURE:
			case TEXTURE_TYPEs::DEPTHTEXTURE:
			case TEXTURE_TYPEs::DEPTHSTENCIL:
				return Find_RTTexture_FormatChannel(TEXTURE->Properties.CHANNEL_TYPE);
			case TEXTURE_TYPEs::COLOR_TEXTURE:
			case TEXTURE_TYPEs::OPACITYTEXTURE:
				return Find_SamplerTexture_Format(TEXTURE->Properties.CHANNEL_TYPE);
			default:
				LOG_ERROR("Find_glTexImage2D_InternalFormat doesn't support this Texture_Type!");
			}
		}
		//Sampler Texture
		else if (TEXTURE->OP_TYPE == BUFFER_VISIBILITY::CPUEXISTENCE_GPUREADONLY) {
			return Find_SamplerTexture_Format(TEXTURE->Properties.CHANNEL_TYPE);
		}
		LOG_ERROR("Find_glTexImage2D_InternalFormat doesn't support this BUFFER_VISIBILITY!");
	}


	unsigned int Find_SamplerTexture_Format(TEXTURE_CHANNELs channels) {
		switch (channels) {
		case TEXTURE_CHANNELs::API_TEXTURE_RGB32F:
		case TEXTURE_CHANNELs::API_TEXTURE_RGB32I:
		case TEXTURE_CHANNELs::API_TEXTURE_RGB32UI:
		case TEXTURE_CHANNELs::API_TEXTURE_RGB8B:
		case TEXTURE_CHANNELs::API_TEXTURE_RGB8UB:
			return GL_RGB;
		case TEXTURE_CHANNELs::API_TEXTURE_RGBA32F:
		case TEXTURE_CHANNELs::API_TEXTURE_RGBA32I:
		case TEXTURE_CHANNELs::API_TEXTURE_RGBA32UI:
		case TEXTURE_CHANNELs::API_TEXTURE_RGBA8B:
		case TEXTURE_CHANNELs::API_TEXTURE_RGBA8UB:
			return GL_RGBA;
		case TEXTURE_CHANNELs::API_TEXTURE_R32F:
		case TEXTURE_CHANNELs::API_TEXTURE_R32I:
		case TEXTURE_CHANNELs::API_TEXTURE_R32UI:
		case TEXTURE_CHANNELs::API_TEXTURE_R8B:
		case TEXTURE_CHANNELs::API_TEXTURE_R8UB:
			return GL_ALPHA;
		case TEXTURE_CHANNELs::API_TEXTURE_RA32F:
		case TEXTURE_CHANNELs::API_TEXTURE_RA32I:
		case TEXTURE_CHANNELs::API_TEXTURE_RA32UI:
		case TEXTURE_CHANNELs::API_TEXTURE_RA8B:
		case TEXTURE_CHANNELs::API_TEXTURE_RA8UB:
			return GL_RG;
		default:
			LOG_CRASHING("Intended texture channel type isn't supported by OpenGL4::Find_glTexImage2D_ChannelType for now!");
			return NULL;
		}
	}

	unsigned int Find_RTTexture_FormatChannel(TEXTURE_CHANNELs channels) {
		switch (channels) {
		case TEXTURE_CHANNELs::API_TEXTURE_RGBA32F:
		case TEXTURE_CHANNELs::API_TEXTURE_RGBA32I:
		case TEXTURE_CHANNELs::API_TEXTURE_RGBA32UI:
		case TEXTURE_CHANNELs::API_TEXTURE_RGBA8B:
		case TEXTURE_CHANNELs::API_TEXTURE_RGBA8UB:
			return GL_RGBA;
		case TEXTURE_CHANNELs::API_TEXTURE_R32F:
		case TEXTURE_CHANNELs::API_TEXTURE_R32I:
		case TEXTURE_CHANNELs::API_TEXTURE_R32UI:
		case TEXTURE_CHANNELs::API_TEXTURE_R8B:
		case TEXTURE_CHANNELs::API_TEXTURE_R8UB:
			return GL_DEPTH_COMPONENT;
		default:
			LOG_CRASHING("Intended texture channel type isn't supported by OpenGL4::Find_glTexImage2D_FormatType for now!");
			return NULL;
		}
	}

	unsigned int Find_ImageTexture_InternalFormat(TEXTURE_CHANNELs channels) {
		switch (channels) {
		case TEXTURE_CHANNELs::API_TEXTURE_RGBA32F:
			return GL_RGBA32F;
		case TEXTURE_CHANNELs::API_TEXTURE_RGBA32I:
			return GL_RGBA32I;
		case TEXTURE_CHANNELs::API_TEXTURE_RGBA32UI:
			return GL_RGBA32UI;
		case TEXTURE_CHANNELs::API_TEXTURE_RGBA8B:
			return GL_RGBA8I;
		case TEXTURE_CHANNELs::API_TEXTURE_RGBA8UB:
			return GL_RGBA8UI;
		case TEXTURE_CHANNELs::API_TEXTURE_R32F:
			return GL_R32F;
		case TEXTURE_CHANNELs::API_TEXTURE_R32I:
			return GL_R32I;
		case TEXTURE_CHANNELs::API_TEXTURE_R32UI:
			return GL_R32UI;
		case TEXTURE_CHANNELs::API_TEXTURE_R8B:
			return GL_R8I;
		case TEXTURE_CHANNELs::API_TEXTURE_R8UB:
			return GL_R8UI;
		default:
			LOG_CRASHING("Intended texture channel type isn't supported by OpenGL4::Find_glTexImage2D_FormatType for now!");
			return NULL;
		}
	}

	unsigned int OGL4_API Find_glTexImage2D_ValueType(GFX_API::TEXTURE_CHANNELs channel_type) {
		switch (channel_type) {
		case TEXTURE_CHANNELs::API_TEXTURE_R8UB:
		case TEXTURE_CHANNELs::API_TEXTURE_RA8UB:
		case TEXTURE_CHANNELs::API_TEXTURE_RGB8UB:
		case TEXTURE_CHANNELs::API_TEXTURE_RGBA8UB:
			return GL_UNSIGNED_BYTE;
		case TEXTURE_CHANNELs::API_TEXTURE_R8B:
		case TEXTURE_CHANNELs::API_TEXTURE_RA8B:
		case TEXTURE_CHANNELs::API_TEXTURE_RGB8B:
		case TEXTURE_CHANNELs::API_TEXTURE_RGBA8B:
			return GL_BYTE;
		case TEXTURE_CHANNELs::API_TEXTURE_R32F:
		case TEXTURE_CHANNELs::API_TEXTURE_RA32F:
		case TEXTURE_CHANNELs::API_TEXTURE_RGB32F:
		case TEXTURE_CHANNELs::API_TEXTURE_RGBA32F:
			return GL_FLOAT;
		case TEXTURE_CHANNELs::API_TEXTURE_R32UI:
		case TEXTURE_CHANNELs::API_TEXTURE_RA32UI:
		case TEXTURE_CHANNELs::API_TEXTURE_RGB32UI:
		case TEXTURE_CHANNELs::API_TEXTURE_RGBA32UI:
			return GL_UNSIGNED_INT;
		case TEXTURE_CHANNELs::API_TEXTURE_R32I:
		case TEXTURE_CHANNELs::API_TEXTURE_RA32I:
		case TEXTURE_CHANNELs::API_TEXTURE_RGB32I:
		case TEXTURE_CHANNELs::API_TEXTURE_RGBA32I:
			return GL_INT;
		default:
			LOG_CRASHING("Intended texture value type isn't supported by OpenGL4::Find_glTexImage2D_ValueType for now!");
			return NULL;
		}
	}

	unsigned int Find_Texture_Wrapping(TEXTURE_WRAPPING wrapping) {
		switch (wrapping) {
		case TEXTURE_WRAPPING::API_TEXTURE_REPEAT:
			return GL_REPEAT;
		case TEXTURE_WRAPPING::API_TEXTURE_MIRRORED_REPEAT:
			return GL_MIRRORED_REPEAT;
		case TEXTURE_WRAPPING::API_TEXTURE_CLAMP_TO_EDGE:
			return GL_CLAMP_TO_EDGE;
		default:
			LOG_CRASHING("Intended Wrapping Type isn't supported by OpenGL4 for now!");
			return NULL;
		}
	}

	unsigned int Find_Texture_Mipmap_Filtering(TEXTURE_MIPMAPFILTER mipmap_filter) {
		switch (mipmap_filter) {
		case TEXTURE_MIPMAPFILTER::API_TEXTURE_LINEAR_FROM_1MIP:
			return GL_LINEAR;
		case TEXTURE_MIPMAPFILTER::API_TEXTURE_LINEAR_FROM_2MIP:
			return GL_LINEAR_MIPMAP_LINEAR;
		case TEXTURE_MIPMAPFILTER::API_TEXTURE_NEAREST_FROM_1MIP:
			return GL_NEAREST;
		case TEXTURE_MIPMAPFILTER::API_TEXTURE_NEAREST_FROM_2MIP:
			return GL_NEAREST_MIPMAP_LINEAR;
		default:
			LOG_CRASHING("Intended Mipmap Filtering Type isn't supported by OpenGL4 for now!");
			return NULL;
		}
	}
	unsigned int Find_ShaderStage(GFX_API::SHADER_STAGE shader_stage) {
		switch (shader_stage) {
		case GFX_API::SHADER_STAGE::VERTEXSTAGE:
			return GL_VERTEX_SHADER;
		case GFX_API::SHADER_STAGE::FRAGMENTSTAGE:
			return GL_FRAGMENT_SHADER;
		default:
			LOG_CRASHING("Intended Shader Stage Type isn't supported by OpenGL4 for now!");
			return NULL;
		}
	}


	unsigned int Find_RenderTarget_AttachmentType(RT_ATTACHMENTs attachment_type) {
		switch (attachment_type) {
		case RT_ATTACHMENTs::TEXTURE_ATTACHMENT_COLOR0:
			return GL_COLOR_ATTACHMENT0;
		case RT_ATTACHMENTs::TEXTURE_ATTACHMENT_DEPTH:
			return GL_DEPTH_ATTACHMENT;
		case RT_ATTACHMENTs::TEXTURE_ATTACHMENT_DEPTHSTENCIL:
			return GL_DEPTH_STENCIL_ATTACHMENT;
		default:
			LOG_CRASHING("Intended Render Target attachment type isn't supported by OpenGL4 for now!", true);
			return NULL;
		}
	}
	//INPUT HANDLING!


	KEYBOARD_KEYs Convert_Key_to_Engine(int GLFW_Key) {
		switch (GLFW_Key) {
		case GLFW_KEY_W:
			return KEYBOARD_KEYs::KEYBOARD_W;
		case GLFW_KEY_S:
			return KEYBOARD_KEYs::KEYBOARD_S;
		case GLFW_KEY_A:
			return KEYBOARD_KEYs::KEYBOARD_A;
		case GLFW_KEY_D:
			return KEYBOARD_KEYs::KEYBOARD_D;
		case GLFW_KEY_KP_2:
			return KEYBOARD_KEYs::KEYBOARD_NP_2;
		case GLFW_KEY_KP_4:
			return KEYBOARD_KEYs::KEYBOARD_NP_4;
		case GLFW_KEY_KP_6:
			return KEYBOARD_KEYs::KEYBOARD_NP_6;
		case GLFW_KEY_KP_8:
			return KEYBOARD_KEYs::KEYBOARD_NP_8;
		case GLFW_KEY_C:
			return KEYBOARD_KEYs::KEYBOARD_C;

		default:
			"Error: Intended key isn't supported, returns KEYBOARD_A!\n";
			return KEYBOARD_KEYs::KEYBOARD_A;
		}
	}

	int Convert_Key_to_GLFW_Key(KEYBOARD_KEYs Keyboard_Key) {
		switch (Keyboard_Key) {
		case KEYBOARD_KEYs::KEYBOARD_W:
			return GLFW_KEY_W;
		case KEYBOARD_KEYs::KEYBOARD_S:
			return GLFW_KEY_S;
		case KEYBOARD_KEYs::KEYBOARD_A:
			return GLFW_KEY_A;
		case KEYBOARD_KEYs::KEYBOARD_D:
			return GLFW_KEY_D;
		case KEYBOARD_KEYs::KEYBOARD_NP_2:
			return GLFW_KEY_KP_2;
		case KEYBOARD_KEYs::KEYBOARD_NP_4:
			return GLFW_KEY_KP_4;
		case KEYBOARD_KEYs::KEYBOARD_NP_6:
			return GLFW_KEY_KP_6;
		case KEYBOARD_KEYs::KEYBOARD_NP_8:
			return GLFW_KEY_KP_8;
		case KEYBOARD_KEYs::KEYBOARD_C:
			return GLFW_KEY_C;

		default:
			cout << "Error: Intended key isn't supported, return GLFW_KEY_A!\n";
			return GLFW_KEY_A;
		}
	}


	int Convert_MouseButton_to_GLFW_Key(MOUSE_BUTTONs Mouse_Button) {
		switch (Mouse_Button) {
		case MOUSE_BUTTONs::MOUSE_LEFT_CLICK:
			return GLFW_MOUSE_BUTTON_LEFT;
		case MOUSE_BUTTONs::MOUSE_RIGHT_CLICK:
			return GLFW_MOUSE_BUTTON_RIGHT;
		case MOUSE_BUTTONs::MOUSE_WHEEL_CLICK:
			return GLFW_MOUSE_BUTTON_MIDDLE;

		default:
			cout << "Error: Intended mouse button isn't supported, return GLFW_MOUSE_BUTTON_1!\n";
			return GLFW_MOUSE_BUTTON_1;
		}
	}

	MOUSE_BUTTONs Convert_GLFWButton_toGFXButton(int button) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			return MOUSE_BUTTONs::MOUSE_LEFT_CLICK;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			return MOUSE_BUTTONs::MOUSE_WHEEL_CLICK;
		case GLFW_MOUSE_BUTTON_RIGHT:
			return MOUSE_BUTTONs::MOUSE_RIGHT_CLICK;
		default:
			LOG_CRASHING("Convert_GLFWButton_toGFXButton() doesn't support this type of mouse button!");
			return MOUSE_BUTTONs::MOUSE_LEFT_CLICK;
		}
	}



	unsigned int Find_OGLDepthTest(DEPTH_TESTs depth_test) {
		switch (depth_test) {
		case DEPTH_TESTs::DEPTH_TEST_ALWAYS:
			return GL_ALWAYS;
		case DEPTH_TESTs::DEPTH_TEST_GEQUAL:
			return GL_GEQUAL;
		case DEPTH_TESTs::DEPTH_TEST_GREATER:
			return GL_GREATER;
		case DEPTH_TESTs::DEPTH_TEST_LEQUAL:
			return GL_LEQUAL;
		case DEPTH_TESTs::DEPTH_TEST_LESS:
			return GL_LESS;
		case DEPTH_TESTs::DEPTH_TEST_NEVER:
			return GL_NEVER;
		default:
			LOG_NOTCODED("Intended Depth Test Mode can't be found! GL_NEVER is returned!", true);
			return GL_NEVER;
		}
	}
	


	unsigned int OGL4_API Find_BUFFERUSAGE(GFX_API::BUFFER_VISIBILITY usage) {
		switch (usage) {
		case BUFFER_VISIBILITY::CPUEXISTENCE_GPUREADONLY:
		case BUFFER_VISIBILITY::CPUEXISTENCE_GPUREADWRITE:
		case BUFFER_VISIBILITY::CPUREADONLY_GPUREADWRITE:
			return GL_STATIC_DRAW;
		case BUFFER_VISIBILITY::CPUREADWRITE_GPUREADONLY:
		case BUFFER_VISIBILITY::CPUREADWRITE_GPUREADWRITE:
			return GL_DYNAMIC_DRAW;
		default:
			LOG_NOTCODED("Find_BUFFERUSAGE doesn't support this usage!", true);
		}
	}

	unsigned int OGL4_API Find_OGLOperationType(GFX_API::OPERATION_TYPE operation_type) {
		switch (operation_type) {
		case OPERATION_TYPE::READ_ONLY:
			return GL_READ_ONLY;
		case OPERATION_TYPE::READ_AND_WRITE:
			return GL_READ_WRITE;
		case OPERATION_TYPE::WRITE_ONLY:
			return GL_WRITE_ONLY;
		default:
			LOG_ERROR("Find_OGLOperationType doesn't support this operation type!");
		}
	}

	void Check_ActiveFramebuffer_Status(const char* Framebuffer_Name) {
		string log;
		int fb_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (fb_status == GL_FRAMEBUFFER_COMPLETE) {
			log.append("Framebuffer is completed, Name: ");
			log.append(Framebuffer_Name);
			LOG_STATUS(log);
		}
		else if (fb_status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
			log.append("Framebuffer has incomplete attachment, Name: ");
			log.append(Framebuffer_Name);
			LOG_CRASHING(log);
		}
		else if (fb_status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
			log.append("Framebuffer has incomplete missing attachment, Name: ");
			log.append(Framebuffer_Name);
			LOG_CRASHING(log);
		}
		else if (fb_status == GL_FRAMEBUFFER_UNSUPPORTED) {
			log.append("Framebuffer has unsupported type of attachment, Name: ");
			log.append(Framebuffer_Name);
			LOG_CRASHING(log);
		}
	}
}
