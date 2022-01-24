#pragma once
#include "OpenGL4/OpenGL4_Includes.h"
#include "GFX/Renderer/GPU_ContentManager.h"
#include "GFX/Renderer/GFX_Renderer.h"

namespace OpenGL4 {

	//OGL4_MESH represents both Meshes' and Points' data because both of them uses VAO and VBO. 
	//Also PointBuffer may use MeshBuffer's first vertex attribute, that means there is no need for new vertex attribute
	struct OGL4_API OGL4_MESH {
		unsigned int VAO = 0, VBO = 0, EBO = 0;
	};

	struct OGL4_API OGL4_ComputeShader {
		unsigned int ComputeShader_ID, ShaderProgram_ID;
	};


	class OGL4_API GPU_ContentManager : public GFX_API::GPU_ContentManager {
		//I need to bind Renderer's context before sending any data, that's why I store pointer
		GFX_API::Renderer* RENDERER;

		//GLSL uses binding points to access uniform buffers and binding points are unsigned int.
		Bitset UNIFORMBUFFER_BINDINGPOINTs;
		unsigned int Create_BindingPoint();
		void Delete_BindingPoint(unsigned int ID);
		friend class OpenGL4_Core;
	public:
		GPU_ContentManager();
		virtual ~GPU_ContentManager();
		virtual void Unload_AllResources() override;

		//Return MeshBufferID to use in Draw Calls
		virtual unsigned int Upload_MeshBuffer(const GFX_API::VertexAttributeLayout& attributelayout, const void* vertex_data, unsigned int data_size, unsigned int vertex_count,
			const void* index_data, unsigned int index_count) override;
		//When you call this function, Draw Calls that uses this ID may draw another Mesh or crash
		//Also if you have any Point Buffer that uses first vertex attribute of that Mesh Buffer, it may crash or draw any other buffer
		virtual void Unload_MeshBuffer(unsigned int MeshBuffer_ID) override;


		virtual unsigned int Create_PointBuffer(const GFX_API::VertexAttributeLayout& attributelayout, const void* point_data, unsigned int point_count) override;
		virtual void Upload_PointBuffer(unsigned int PointBufferID, const void* point_data) override;
		virtual unsigned int CreatePointBuffer_fromMeshBuffer(unsigned int MeshBuffer_ID, unsigned int AttributeIndex_toUseAsPointBuffer) override;
		virtual void Unload_PointBuffer(unsigned int PointBuffer_ID) override;


		virtual void Create_Texture(GFX_API::Texture_Resource* TEXTURE_ASSET, unsigned int Asset_ID) override;
		virtual void Upload_Texture(unsigned int Asset_ID, void* DATA, unsigned int DATA_SIZE) override;
		virtual void Unload_Texture(unsigned int ASSET_ID) override;


		virtual unsigned int Create_GlobalBuffer(const char* BUFFER_NAME, void* DATA, unsigned int DATA_SIZE, GFX_API::BUFFER_VISIBILITY USAGE) override;
		virtual void Upload_GlobalBuffer(unsigned int BUFFER_ID, const void* DATA = nullptr, unsigned int DATA_SIZE = 0) override;
		virtual const void* StartReading_GlobalBuffer(unsigned int BufferID, GFX_API::OPERATION_TYPE optype) override;
		virtual void FinishReading_GlobalBuffer(unsigned int BufferID) override;
		virtual void Unload_GlobalBuffer(unsigned int BUFFER_ID) override;


		virtual void Compile_ShaderSource(GFX_API::ShaderSource_Resource* SHADER, unsigned int Asset_ID, string* compilation_status) override;
		virtual void Delete_ShaderSource(unsigned int ASSET_ID) override;
		virtual void Compile_ComputeShader(GFX_API::ComputeShader_Resource* SHADER, unsigned int Asset_ID, string* compilation_status) override;
		virtual void Delete_ComputeShader(unsigned int ASSET_ID) override;
		virtual void Link_MaterialType(GFX_API::Material_Type* MATTYPE_ASSET, unsigned int Asset_ID, string* compilation_status) override;
		virtual void Delete_MaterialType(unsigned int Asset_ID) override;


		virtual unsigned int Create_Framebuffer() override;
		virtual void Attach_RenderTarget_toFramebuffer(const GFX_API::Framebuffer::RT_SLOT* RT_SLOT, GFX_API::RT_ATTACHMENTs ATTACHMENT_TYPE, unsigned int FB_ID) override;
		virtual void* ReadFramebufferAttachment(GFX_API::RT_ATTACHMENTs ATTACHMENT_TYPE, unsigned int FB_ID, unsigned int& DATASIZE) override;
		virtual void Delete_Framebuffer(unsigned int Framebuffer_ID) override;
	};


}