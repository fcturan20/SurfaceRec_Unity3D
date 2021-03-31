#include "VK_GPUContentManager.h"

namespace Vulkan {
	GPU_ContentManager::GPU_ContentManager() {

	}
	GPU_ContentManager::~GPU_ContentManager() {

	}
	void GPU_ContentManager::Unload_AllResources() {

	}

	//Return MeshBufferID to use in Draw Calls
	unsigned int GPU_ContentManager::Upload_MeshBuffer(const GFX_API::VertexAttributeLayout& attributelayout, const void* vertex_data, unsigned int data_size, unsigned int vertex_count,
		const void* index_data, unsigned int index_count) {
		TuranAPI::LOG_NOTCODED("VK::Upload_MeshBuffer isn't coded!", true);
	}
	//When you call this function, Draw Calls that uses this ID may draw another Mesh or crash
	//Also if you have any Point Buffer that uses first vertex attribute of that Mesh Buffer, it may crash or draw any other buffer
	void GPU_ContentManager::Unload_MeshBuffer(unsigned int MeshBuffer_ID) {
		TuranAPI::LOG_NOTCODED("VK::Unload_MeshBuffer isn't coded!", true);
	}


	unsigned int GPU_ContentManager::Upload_PointBuffer(const void* point_data, unsigned int data_size, unsigned int point_count) {
		TuranAPI::LOG_NOTCODED("VK::Upload_PointBuffer isn't coded!", true);
	}
	unsigned int GPU_ContentManager::CreatePointBuffer_fromMeshBuffer(unsigned int MeshBuffer_ID, unsigned int AttributeIndex_toUseAsPointBuffer) {
		TuranAPI::LOG_NOTCODED("VK::CreatePointBuffer_fromMeshBuffer isn't coded!", true);
	}
	void GPU_ContentManager::Unload_PointBuffer(unsigned int PointBuffer_ID) {
		TuranAPI::LOG_NOTCODED("VK::Unload_PointBuffer isn't coded!", true);
	}


	void GPU_ContentManager::Create_Texture(GFX_API::Texture_Resource* TEXTURE_ASSET, unsigned int Asset_ID) {
		TuranAPI::LOG_NOTCODED("VK::Create_Texture isn't coded!", true);
	}
	void GPU_ContentManager::Upload_Texture(unsigned int Asset_ID, void* DATA, unsigned int DATA_SIZE){
		TuranAPI::LOG_NOTCODED("VK::Upload_Texture isn't coded!", true);
	}
	void GPU_ContentManager::Unload_Texture(unsigned int ASSET_ID) {
		TuranAPI::LOG_NOTCODED("VK::Unload_Texture isn't coded!", true);
	}


	unsigned int GPU_ContentManager::Create_GlobalBuffer(const char* BUFFER_NAME, void* DATA, unsigned int DATA_SIZE, GFX_API::BUFFER_VISIBILITY USAGE) {
		TuranAPI::LOG_NOTCODED("VK::Create_GlobalBuffer isn't coded!", true);
	}
	void GPU_ContentManager::Upload_GlobalBuffer(unsigned int BUFFER_ID, void* DATA = nullptr, unsigned int DATA_SIZE = 0) {
		TuranAPI::LOG_NOTCODED("VK::Upload_GlobalBuffer isn't coded!", true);
	}
	void GPU_ContentManager::Unload_GlobalBuffer(unsigned int BUFFER_ID) {
		TuranAPI::LOG_NOTCODED("VK::Unload_GlobalBuffer isn't coded!", true);
	}


	void GPU_ContentManager::Compile_ShaderSource(GFX_API::ShaderSource_Resource* SHADER, unsigned int Asset_ID, string* compilation_status) {
		TuranAPI::LOG_NOTCODED("VK::Unload_GlobalBuffer isn't coded!", true);
	}
	void GPU_ContentManager::Delete_ShaderSource(unsigned int ASSET_ID) {
		TuranAPI::LOG_NOTCODED("VK::Unload_GlobalBuffer isn't coded!", true);
	}
	void GPU_ContentManager::Compile_ComputeShader(GFX_API::ComputeShader_Resource* SHADER, unsigned int Asset_ID, string* compilation_status){
		TuranAPI::LOG_NOTCODED("VK::Unload_GlobalBuffer isn't coded!", true);
	}
	void GPU_ContentManager::Delete_ComputeShader(unsigned int ASSET_ID){
		TuranAPI::LOG_NOTCODED("VK::Unload_GlobalBuffer isn't coded!", true);
	}
	void GPU_ContentManager::Link_MaterialType(GFX_API::Material_Type* MATTYPE_ASSET, unsigned int Asset_ID, string* compilation_status){
		TuranAPI::LOG_NOTCODED("VK::Unload_GlobalBuffer isn't coded!", true);
	}
	void GPU_ContentManager::Delete_MaterialType(unsigned int Asset_ID){
		TuranAPI::LOG_NOTCODED("VK::Unload_GlobalBuffer isn't coded!", true);
	}


	unsigned int GPU_ContentManager::Create_Framebuffer(){
		TuranAPI::LOG_NOTCODED("VK::Unload_GlobalBuffer isn't coded!", true);
	}
	void GPU_ContentManager::Attach_RenderTarget_toFramebuffer(const GFX_API::Framebuffer::RT_SLOT* RT_SLOT, GFX_API::RT_ATTACHMENTs ATTACHMENT_TYPE, unsigned int FB_ID){
		TuranAPI::LOG_NOTCODED("VK::Unload_GlobalBuffer isn't coded!", true);
	}
	void GPU_ContentManager::Delete_Framebuffer(unsigned int Framebuffer_ID){
		TuranAPI::LOG_NOTCODED("VK::Unload_GlobalBuffer isn't coded!", true);
	}
}