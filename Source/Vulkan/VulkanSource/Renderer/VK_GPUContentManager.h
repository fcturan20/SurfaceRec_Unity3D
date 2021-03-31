#pragma once
#include "Vulkan/VulkanSource/Vulkan_Includes.h"


namespace Vulkan {
	struct VK_API VK_Texture {
		VkImage Image = {};
		VkImageView ImageView = {};
	};

	struct VK_API VK_ShaderPipeline {
		vector<VkPipelineShaderStageCreateInfo> ShaderStageCreateInfos;
		vector<VkShaderModule> ShaderModules;
	};

	class VK_API GPU_ContentManager : public GFX_API::GPU_ContentManager {
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


		virtual unsigned int Upload_PointBuffer(const void* point_data, unsigned int data_size, unsigned int point_count) override;
		virtual unsigned int CreatePointBuffer_fromMeshBuffer(unsigned int MeshBuffer_ID, unsigned int AttributeIndex_toUseAsPointBuffer) override;
		virtual void Unload_PointBuffer(unsigned int PointBuffer_ID) override;


		virtual void Create_Texture(GFX_API::Texture_Resource* TEXTURE_ASSET, unsigned int Asset_ID) override;
		virtual void Upload_Texture(unsigned int Asset_ID, void* DATA, unsigned int DATA_SIZE) override;
		virtual void Unload_Texture(unsigned int ASSET_ID) override;


		virtual unsigned int Create_GlobalBuffer(const char* BUFFER_NAME, void* DATA, unsigned int DATA_SIZE, GFX_API::BUFFER_VISIBILITY USAGE) override;
		virtual void Upload_GlobalBuffer(unsigned int BUFFER_ID, void* DATA = nullptr, unsigned int DATA_SIZE = 0) override;
		virtual void Unload_GlobalBuffer(unsigned int BUFFER_ID) override;


		virtual void Compile_ShaderSource(GFX_API::ShaderSource_Resource* SHADER, unsigned int Asset_ID, string* compilation_status) override;
		virtual void Delete_ShaderSource(unsigned int ASSET_ID) override;
		virtual void Compile_ComputeShader(GFX_API::ComputeShader_Resource* SHADER, unsigned int Asset_ID, string* compilation_status) override;
		virtual void Delete_ComputeShader(unsigned int ASSET_ID) override;
		virtual void Link_MaterialType(GFX_API::Material_Type* MATTYPE_ASSET, unsigned int Asset_ID, string* compilation_status) override;
		virtual void Delete_MaterialType(unsigned int Asset_ID) override;


		virtual unsigned int Create_Framebuffer() override;
		virtual void Attach_RenderTarget_toFramebuffer(const GFX_API::Framebuffer::RT_SLOT* RT_SLOT, GFX_API::RT_ATTACHMENTs ATTACHMENT_TYPE, unsigned int FB_ID) override;
		virtual void Delete_Framebuffer(unsigned int Framebuffer_ID) override;
	};
}