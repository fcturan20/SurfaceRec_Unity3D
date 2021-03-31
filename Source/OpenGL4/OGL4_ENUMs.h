#pragma once
#include "GFX/GFX_Core.h"
#include "OpenGL4_Includes.h"


namespace OpenGL4 {
	unsigned int OGL4_API Find_Texture_Dimension(GFX_API::TEXTURE_DIMENSIONs dimension);
	unsigned int OGL4_API Find_glTexImage2D_InternalFormat(GFX_API::Texture_Resource* TEXTURE);
	unsigned int OGL4_API Find_glTexImage2D_Format(GFX_API::Texture_Resource* TEXTURE);
	unsigned int OGL4_API Find_glTexImage2D_ValueType(GFX_API::TEXTURE_CHANNELs channel_type);
	unsigned int OGL4_API Find_ImageTexture_InternalFormat(GFX_API::TEXTURE_CHANNELs channel_type);
	unsigned int OGL4_API Find_Texture_Wrapping(GFX_API::TEXTURE_WRAPPING wrapping);
	unsigned int OGL4_API Find_Texture_Mipmap_Filtering(GFX_API::TEXTURE_MIPMAPFILTER mipmap_filter);

	unsigned int OGL4_API Find_ShaderStage(GFX_API::SHADER_STAGE shader_stage);

	unsigned int OGL4_API Find_RenderTarget_AttachmentType(GFX_API::RT_ATTACHMENTs texture_attachment);

	unsigned int OGL4_API Find_OGLOperationType(GFX_API::OPERATION_TYPE operation_type);

	GFX_API::KEYBOARD_KEYs OGL4_API Convert_Key_to_Engine(int GLFW_Key);
	int OGL4_API Convert_Key_to_GLFW_Key(GFX_API::KEYBOARD_KEYs Keyboard_Key);
	int OGL4_API Convert_MouseButton_to_GLFW_Key(GFX_API::MOUSE_BUTTONs Mouse_Button);
	GFX_API::MOUSE_BUTTONs Convert_GLFWButton_toGFXButton(int button);

	unsigned int OGL4_API Find_OGLDepthTest(GFX_API::DEPTH_TESTs depth_test);

	unsigned int OGL4_API Find_BUFFERUSAGE(GFX_API::BUFFER_VISIBILITY usage);
	void Check_ActiveFramebuffer_Status(const char* Framebuffer_Name);
}
