#include "OGL4_GPUContentManager.h"
#include "OpenGL4/OGL4_ENUMs.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <string>

namespace OpenGL4 {
	//If there is at least one Continuous Attribute before, that means there should be a Start_Offset in one of the them!
	size_t OGL4_API Get_WherePreviousAttribute_Ends(const GFX_API::VertexAttributeLayout* attributelayout, size_t vertex_count, unsigned int currentattribute_index) {
		if (currentattribute_index == 0) {
			return 0;
		}
		const GFX_API::VertexAttribute& previous_attribute = attributelayout->Attributes[currentattribute_index - 1];
		if (previous_attribute.Start_Offset == 0) {
			return Get_WherePreviousAttribute_Ends(attributelayout, vertex_count, currentattribute_index - 1) + (GFX_API::Get_UNIFORMTYPEs_SIZEinbytes(previous_attribute.DATATYPE) * vertex_count);
		}
		else {
			return previous_attribute.Start_Offset + (GFX_API::Get_UNIFORMTYPEs_SIZEinbytes(previous_attribute.DATATYPE) * vertex_count);
		}
	}
	void OGL4_API Set_VertexAttribPointer(const GFX_API::VertexAttribute* attribute, size_t Start_Offset) {
		switch (attribute->DATATYPE)
		{
		case GFX_API::DATA_TYPE::VAR_FLOAT32:
			glVertexAttribPointer(attribute->Index, 1, GL_FLOAT, GL_FALSE, sizeof(float32), (void*)(Start_Offset));
			break;
		case GFX_API::DATA_TYPE::VAR_INT32:
			glVertexAttribPointer(attribute->Index, 1, GL_INT, GL_FALSE, sizeof(int), (void*)(Start_Offset));
			break;
		case GFX_API::DATA_TYPE::VAR_UINT32:
			glVertexAttribPointer(attribute->Index, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(unsigned int), (void*)(Start_Offset));
			break;
		case GFX_API::DATA_TYPE::VAR_BYTE8:
			glVertexAttribPointer(attribute->Index, 1, GL_BYTE, GL_FALSE, sizeof(char), (void*)(Start_Offset));
			break;
		case GFX_API::DATA_TYPE::VAR_UBYTE8:
			glVertexAttribPointer(attribute->Index, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(unsigned char), (void*)(Start_Offset));
			break;
		case GFX_API::DATA_TYPE::VAR_VEC2:
			glVertexAttribPointer(attribute->Index, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)(Start_Offset));
			break;
		case GFX_API::DATA_TYPE::VAR_VEC3:
			glVertexAttribPointer(attribute->Index, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)(Start_Offset));
			break;
		case GFX_API::DATA_TYPE::VAR_VEC4:
			glVertexAttribPointer(attribute->Index, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)(Start_Offset));
			break;
		case GFX_API::DATA_TYPE::VAR_MAT4x4:
			glVertexAttribPointer(attribute->Index, 16, GL_FLOAT, GL_FALSE, sizeof(mat4x4), (void*)(Start_Offset));
			break;
		default:
			LOG_CRASHING("Uniform Type isn't supported to set to Vertex Attribute!");
			break;
		}
	}
	void Bind_GlobalBuffersto_ShaderProgram(const vector<GFX_API::GlobalBuffer_Access>& GLOBALBUFFERs, unsigned int SHADERPROGRAM) {
		for (unsigned int i = 0; i < GLOBALBUFFERs.size(); i++) {
			GFX_API::GFX_Buffer* BUFFER = GFXContentManager->Find_GlobalBuffer_byBufferID(GLOBALBUFFERs[i].BUFFER_ID);
			unsigned int BUFFER_BINDINGPOINT = 0, buffer_index = 0;
			BUFFER_BINDINGPOINT = *(unsigned int*)BUFFER->BINDING_POINT;

			switch (BUFFER->USAGE) {
			case GFX_API::BUFFER_VISIBILITY::CPUEXISTENCE_GPUREADONLY:
			case GFX_API::BUFFER_VISIBILITY::CPUREADWRITE_GPUREADONLY:
				buffer_index = glGetUniformBlockIndex(SHADERPROGRAM, BUFFER->NAME.c_str());
				glUniformBlockBinding(SHADERPROGRAM, buffer_index, BUFFER_BINDINGPOINT);
				break;
			case GFX_API::BUFFER_VISIBILITY::CPUEXISTENCE_GPUREADWRITE:
			case GFX_API::BUFFER_VISIBILITY::CPUREADWRITE_GPUREADWRITE:
			case GFX_API::BUFFER_VISIBILITY::CPUREADONLY_GPUREADWRITE:
				buffer_index = glGetProgramResourceIndex(SHADERPROGRAM, GL_SHADER_STORAGE_BLOCK, BUFFER->NAME.c_str());
				glShaderStorageBlockBinding(SHADERPROGRAM, buffer_index, BUFFER_BINDINGPOINT);
				break;
			default:
				LOG_NOTCODED("Bind_GlobalBuffersto_ShaderProgram doesn't support this type of buffer usage!", true);
			}
		}
	}

	GPU_ContentManager::GPU_ContentManager() : UNIFORMBUFFER_BINDINGPOINTs(100){

	}
	GPU_ContentManager::~GPU_ContentManager() {
		std::cout << "OpenGL4's GPU_ContentManager Destructor is called!\n";
	}
	void GPU_ContentManager::Unload_AllResources() {
		LOG_STATUS("Unloading the GPU Resources!");
		for (unsigned int i = 0; i < MESHBUFFERs.size(); i++) {
			Unload_MeshBuffer(MESHBUFFERs[i].BUFFER_ID);
		}
		for (unsigned int i = 0; i < POINTBUFFERs.size(); i++) {
			Unload_PointBuffer(POINTBUFFERs[i].BUFFER_ID);
		}
		for (unsigned int i = 0; i < GLOBALBUFFERs.size(); i++) {
			Unload_GlobalBuffer(GLOBALBUFFERs[i].ID);
		}
		for (unsigned int i = 0; i < TEXTUREs.size(); i++) {
			Unload_Texture(TEXTUREs[i].ASSET_ID);
		}
		for (unsigned int i = 0; i < SHADERPROGRAMs.size(); i++) {
			Delete_MaterialType(SHADERPROGRAMs[i].ASSET_ID);
		}
		for (unsigned int i = 0; i < SHADERSOURCEs.size(); i++) {
			Delete_ShaderSource(SHADERSOURCEs[i].ASSET_ID);
		}
		for (unsigned int i = 0; i < FBs.size(); i++) {
			Delete_Framebuffer(FBs[i].ID);
		}
		LOG_STATUS("Unloading has finished!");
	}


	unsigned int GPU_ContentManager::Upload_MeshBuffer(const GFX_API::VertexAttributeLayout& attributelayout, const void* vertex_data, unsigned int data_size, unsigned int vertex_count,
		const void* index_data, unsigned int index_count) {
		
		GFX_API::GFX_Mesh MESH;
		OGL4_MESH* GL_MESH = new OGL4_MESH;
		MESH.GL_ID = GL_MESH;
		MESH.VERTEX_COUNT = vertex_count;
		MESH.INDEX_COUNT = index_count;
		glGenVertexArrays(1, &GL_MESH->VAO);
		glGenBuffers(1, &GL_MESH->VBO);
		glGenBuffers(1, &GL_MESH->EBO);
		glBindVertexArray(GL_MESH->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, GL_MESH->VBO);
		glBufferData(GL_ARRAY_BUFFER, data_size, vertex_data, GL_STATIC_DRAW);

		for (unsigned int attribute_index = 0; attribute_index < attributelayout.Attributes.size(); attribute_index++) {
			const GFX_API::VertexAttribute& attribute = attributelayout.Attributes[attribute_index];

			glEnableVertexAttribArray(attribute.Index);
			size_t Start_Offset = 0;
			if (attribute.Start_Offset == 0) {
				Start_Offset = Get_WherePreviousAttribute_Ends(&attributelayout, vertex_count, attribute_index);
				Set_VertexAttribPointer(&attribute, Start_Offset);
			}
			else {
				Set_VertexAttribPointer(&attribute, attribute.Start_Offset);
			}
		}
		if (index_data) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_MESH->EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * index_count, index_data, GL_STATIC_DRAW);
		}
		MESH.BUFFER_ID = Create_MeshBufferID();
		MESHBUFFERs.push_back(MESH);
		glBindVertexArray(0);
		return MESH.BUFFER_ID;
	}
	void GPU_ContentManager::Unload_MeshBuffer(unsigned int MeshBuffer_ID) {
		GFX_API::GFX_Mesh* MESH = nullptr;
		unsigned int vector_index = 0;
		MESH = Find_MeshBuffer_byBufferID(MeshBuffer_ID, &vector_index);
		if (MESH) {
			OGL4_MESH* OGL_MESH = (OGL4_MESH*)MESH->GL_ID;
			if (OGL_MESH) {
				glDeleteVertexArrays(1, &OGL_MESH->VAO);
				glDeleteBuffers(1, &OGL_MESH->VBO);
				glDeleteBuffers(1, &OGL_MESH->EBO);
				delete OGL_MESH;
			}
			MESH->GL_ID = nullptr;
			MESHBUFFERs.erase(MESHBUFFERs.begin() + vector_index);
			return;
		}

		LOG_WARNING("Unload has failed because intended Mesh Buffer isn't found in OpenGL::GPU_ContentManager!");
	}

	unsigned int GPU_ContentManager::Create_PointBuffer(const GFX_API::VertexAttributeLayout& attributelayout, const void* point_data, unsigned int point_count) {
		GFX_API::GFX_Point POINT;
		OGL4_MESH* GL_MESH = new OGL4_MESH;
		POINT.BUFFER_ID = Create_PointBufferID();
		POINT.GL_ID = GL_MESH;
		POINT.POINT_COUNT = point_count;
		POINT.PointAttributeLayout = attributelayout;

		glGenVertexArrays(1, &GL_MESH->VAO);
		glGenBuffers(1, &GL_MESH->VBO);
		glBindVertexArray(GL_MESH->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, GL_MESH->VBO);
		if (point_data != nullptr) {
			glBufferData(GL_ARRAY_BUFFER, attributelayout.size_pervertex * point_count, point_data, GL_STATIC_DRAW);
		}


		for (unsigned int attribute_index = 0; attribute_index < attributelayout.Attributes.size(); attribute_index++) {
			const GFX_API::VertexAttribute& attribute = attributelayout.Attributes[attribute_index];

			glEnableVertexAttribArray(attribute.Index);
			size_t Start_Offset = 0;
			if (attribute.Start_Offset == 0) {
				Start_Offset = Get_WherePreviousAttribute_Ends(&attributelayout, point_count, attribute_index);
				Set_VertexAttribPointer(&attribute, Start_Offset);
			}
			else {
				Set_VertexAttribPointer(&attribute, attribute.Start_Offset);
			}
		}
		POINTBUFFERs.push_back(POINT);
		glBindVertexArray(0);
		return POINT.BUFFER_ID;
	}
	void GPU_ContentManager::Upload_PointBuffer(unsigned int PointBufferID, const void* point_data) {
		unsigned int i = 0;
		GFX_API::GFX_Point* POINT = Find_PointBuffer_byBufferID(PointBufferID, &i);
		if (!POINT) {
			LOG_CRASHING("Point buffer isn't found!");
			return;
		}
		OGL4_MESH* GLPOINT = (OGL4_MESH*)POINT->GL_ID;
		if (!GLPOINT) {
			LOG_CRASHING("Point buffer's GL representation isn't found!");
			return;
		}
		glBindVertexArray(GLPOINT->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, GLPOINT->VBO);
		glBufferData(GL_ARRAY_BUFFER, POINT->PointAttributeLayout.size_pervertex * POINT->POINT_COUNT, point_data, GL_STATIC_DRAW);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	unsigned int GPU_ContentManager::CreatePointBuffer_fromMeshBuffer(unsigned int MeshBuffer_ID, unsigned int AttributeIndex_toUseAsPointBuffer) {
		GFX_API::GFX_Mesh* MESH = nullptr;
		MESH = Find_MeshBuffer_byBufferID(MeshBuffer_ID);
		if (!MESH) {
			return 0;
		}

		GFX_API::GFX_Point PointBuffer;
		//If first vertex attribute is gonna be used, there is no need to create a Vertex Array-Buffer and upload the data.
		if (AttributeIndex_toUseAsPointBuffer == 0) {
			PointBuffer.GL_ID = MESH->GL_ID;
			PointBuffer.BUFFER_ID = Create_PointBufferID();
			if (MESH->INDEX_COUNT) {
				PointBuffer.POINT_COUNT = MESH->INDEX_COUNT;
			}
			else {
				PointBuffer.POINT_COUNT = MESH->VERTEX_COUNT;
			}
			POINTBUFFERs.push_back(PointBuffer);
			return PointBuffer.BUFFER_ID;
		}
		//First vertex attribute isn't gonna be used, so re-upload the data
		else {
			LOG_NOTCODED("OpenGL4::GPU_ContentManager::CreatePointBuffer_fromMeshBuffer() only supports using the first vertex attribute as Point Buffer for now!", true);
			return 0;
		}

	}
	void GPU_ContentManager::Unload_PointBuffer(unsigned int PointBuffer_ID) {
		GFX_API::GFX_Point* POINT = nullptr;
		unsigned int vector_index = 0;
		POINT = Find_PointBuffer_byBufferID(PointBuffer_ID, &vector_index);
		if (POINT) {
			for (unsigned int i = 0; i < MESHBUFFERs.size(); i++) {
				if (POINT->GL_ID == MESHBUFFERs[i].GL_ID) {		//If the Point Buffer uses a Mesh Buffer's first vertex attribute
					POINT->GL_ID = nullptr;
					POINTBUFFERs.erase(POINTBUFFERs.begin() + vector_index);
					return;
				}
			}
			//If arrives here, that means Point Buffer doesn't use any Mesh Buffer's data or at least doesn't use its first vertex attribute

			if (POINT->GL_ID) {
				OGL4_MESH* POINTBUFFER = (OGL4_MESH*)POINT->GL_ID;
				if (POINTBUFFER) {
					glDeleteVertexArrays(1, &POINTBUFFER->VAO);
					glDeleteBuffers(1, &POINTBUFFER->VBO);
				}
				POINT->GL_ID = nullptr;
				POINTBUFFERs.erase(POINTBUFFERs.begin() + vector_index);
				return;
			}
		}
		LOG_WARNING("Unload has failed because intended Point Buffer isn't found in OpenGL::GPU_ContentManager!");
	}


	void GPU_ContentManager::Create_Texture(GFX_API::Texture_Resource* ASSET, unsigned int Asset_ID) {
		unsigned int TEXTURE_DIMENSION = Find_Texture_Dimension(ASSET->Properties.DIMENSION);
		unsigned int* TEXTUREID = new unsigned int(0);
		glGenTextures(1, TEXTUREID);
		glBindTexture(TEXTURE_DIMENSION, *TEXTUREID);


		unsigned int TEXTURE_WRAPPING = Find_Texture_Wrapping(ASSET->Properties.WRAPPING);
		unsigned int TEXTURE_MIPMAPFILTER = Find_Texture_Mipmap_Filtering(ASSET->Properties.MIPMAP_FILTERING);
		unsigned int TEXTURE_VALUETYPE = Find_glTexImage2D_ValueType(ASSET->Properties.CHANNEL_TYPE);


		glTexParameteri(TEXTURE_DIMENSION, GL_TEXTURE_WRAP_S, TEXTURE_WRAPPING);
		glTexParameteri(TEXTURE_DIMENSION, GL_TEXTURE_WRAP_T, TEXTURE_WRAPPING);
		glTexParameteri(TEXTURE_DIMENSION, GL_TEXTURE_MIN_FILTER, TEXTURE_MIPMAPFILTER);
		glTexParameteri(TEXTURE_DIMENSION, GL_TEXTURE_MAG_FILTER, TEXTURE_MIPMAPFILTER);

		if (ASSET->OP_TYPE == GFX_API::BUFFER_VISIBILITY::CPUEXISTENCE_GPUREADWRITE || ASSET->OP_TYPE == GFX_API::BUFFER_VISIBILITY::CPUEXISTENCE_GPUREADONLY) {
			glTexImage2D(TEXTURE_DIMENSION, 0, Find_glTexImage2D_InternalFormat(ASSET), ASSET->WIDTH, ASSET->HEIGHT, 0, Find_glTexImage2D_Format(ASSET), TEXTURE_VALUETYPE, ASSET->DATA);
			if (ASSET->Has_Mipmaps) {
				glGenerateMipmap(TEXTURE_DIMENSION);
			}
		}


		GFX_API::GFX_Texture TEXTURE;
		TEXTURE.GL_ID = TEXTUREID;
		TEXTURE.ASSET_ID = Asset_ID;
		TEXTUREs.push_back(TEXTURE);
	}
	void GPU_ContentManager::Upload_Texture(unsigned int Asset_ID, void* DATA, unsigned int DATA_SIZE) {
		LOG_NOTCODED("Upload_Texture isn't coded!", true);
	}
	void GPU_ContentManager::Unload_Texture(unsigned int TEXTURE_ID) {
		unsigned int vector_index = 0;
		GFX_API::GFX_Texture* TEXTURE = Find_GFXTexture_byID(TEXTURE_ID, &vector_index);
		if (TEXTURE) {
			if (TEXTURE->GL_ID) {
				glDeleteTextures(1, (unsigned int*)TEXTURE->GL_ID);
				delete (unsigned int*)TEXTURE->GL_ID;
				TEXTURE->GL_ID = nullptr;
			}
			TEXTURE->GL_ID = nullptr;
			TEXTUREs.erase(TEXTUREs.begin() + vector_index);
			return;
		}
		LOG_WARNING("Unload has failed because intended Texture isn't found in OpenGL::GPU_ContentManager!");
	}


	unsigned int GPU_ContentManager::Create_GlobalBuffer(const char* NAME, void* DATA, unsigned int DATA_SIZE, GFX_API::BUFFER_VISIBILITY USAGE) {
		unsigned int* BUFFER_GLID = new unsigned int(0);
		unsigned int* BindingPoint = new unsigned int(Create_BindingPoint());
		switch (USAGE) {
		case GFX_API::BUFFER_VISIBILITY::CPUEXISTENCE_GPUREADONLY:
		case GFX_API::BUFFER_VISIBILITY::CPUREADWRITE_GPUREADONLY:
			glGenBuffers(1, BUFFER_GLID);
			glBindBuffer(GL_UNIFORM_BUFFER, *BUFFER_GLID);
			glBufferData(GL_UNIFORM_BUFFER, DATA_SIZE, DATA, Find_BUFFERUSAGE(USAGE));
			glBindBufferBase(GL_UNIFORM_BUFFER, *BindingPoint, *BUFFER_GLID);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
			break;
		case GFX_API::BUFFER_VISIBILITY::CPUEXISTENCE_GPUREADWRITE:
		case GFX_API::BUFFER_VISIBILITY::CPUREADWRITE_GPUREADWRITE:
		case GFX_API::BUFFER_VISIBILITY::CPUREADONLY_GPUREADWRITE:
			glGenBuffers(1, BUFFER_GLID);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, *BUFFER_GLID);
			glBufferData(GL_SHADER_STORAGE_BUFFER, DATA_SIZE, DATA, Find_BUFFERUSAGE(USAGE));
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, *BindingPoint, *BUFFER_GLID);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			break;
		}

		GFX_API::GFX_Buffer BUFFER;
		BUFFER.DATA = DATA;
		BUFFER.DATA_SIZE = DATA_SIZE;
		BUFFER.ID = Create_GlobalBufferID();
		BUFFER.USAGE = USAGE;
		BUFFER.GL_ID = BUFFER_GLID;
		BUFFER.BINDING_POINT = BindingPoint;
		BUFFER.NAME = NAME;
		GLOBALBUFFERs.push_back(BUFFER);
		return BUFFER.ID;
	}
	void GPU_ContentManager::Upload_GlobalBuffer(unsigned int BUFFER_ID, const void* DATA, unsigned int DATA_SIZE) {
		GFX_API::GFX_Buffer* BUFFER = Find_GlobalBuffer_byBufferID(BUFFER_ID);
		if (BUFFER) {
			if (BUFFER->GL_ID) {
				if (DATA) {
					BUFFER->DATA = DATA;
				}
				if (DATA_SIZE) {
					BUFFER->DATA_SIZE = DATA_SIZE;
				}
				switch (BUFFER->USAGE) {
				case GFX_API::BUFFER_VISIBILITY::CPUEXISTENCE_GPUREADONLY:
				case GFX_API::BUFFER_VISIBILITY::CPUREADWRITE_GPUREADONLY:
					glBindBuffer(GL_UNIFORM_BUFFER, *(unsigned int*)BUFFER->GL_ID);
					glBufferData(GL_UNIFORM_BUFFER, BUFFER->DATA_SIZE, BUFFER->DATA, Find_BUFFERUSAGE(BUFFER->USAGE));
					break;
				case GFX_API::BUFFER_VISIBILITY::CPUEXISTENCE_GPUREADWRITE:
				case GFX_API::BUFFER_VISIBILITY::CPUREADWRITE_GPUREADWRITE:
				case GFX_API::BUFFER_VISIBILITY::CPUREADONLY_GPUREADWRITE:
					glBindBuffer(GL_SHADER_STORAGE_BUFFER, *(unsigned int*)BUFFER->GL_ID);
					glBufferData(GL_SHADER_STORAGE_BUFFER, BUFFER->DATA_SIZE, BUFFER->DATA, Find_BUFFERUSAGE(BUFFER->USAGE));
					break;
				default:
					LOG_NOTCODED("Upload_GlobalBuffer doesn't support this type of buffer usage!", true);
				}
			}
			else {
				LOG_ERROR("You shouldn't call Upload_GlobalBuffer(), if you didn't create it with Create_GlobalBuffer()!");
				return;
			}
		}
	}
	void GPU_ContentManager::Unload_GlobalBuffer(unsigned int BUFFER_ID) {
		GFX_API::GFX_Buffer* BUFFER = nullptr;
		unsigned int vector_index = 0;
		BUFFER = Find_GlobalBuffer_byBufferID(BUFFER_ID, &vector_index);
		if (BUFFER) {
			if (BUFFER->GL_ID) {
				glDeleteBuffers(1, (unsigned int*)BUFFER->GL_ID);
				delete (unsigned int*)BUFFER->GL_ID;
				BUFFER->GL_ID = nullptr;
			}
			GLOBALBUFFERs.erase(GLOBALBUFFERs.begin() + vector_index);
			return;
		}
		LOG_WARNING("Unload has failed because intended Global Buffer isn't found in OpenGL::GPU_ContentManager!");
	}



	void GPU_ContentManager::Compile_ShaderSource(GFX_API::ShaderSource_Resource* SHADER, unsigned int Asset_ID, string* compilation_status) {
		unsigned int STAGE = Find_ShaderStage(SHADER->STAGE);

		unsigned int* SHADER_o = new unsigned int(0);
		*SHADER_o = glCreateShader(STAGE);
		auto x = SHADER->SOURCE_CODE.c_str();
		glShaderSource(*SHADER_o, 1, &x, NULL);
		glCompileShader(*SHADER_o);

		//Check compile issues!
		int success;
		char vert_infolog[512];
		glGetShaderiv(*SHADER_o, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(*SHADER_o, 512, NULL, vert_infolog);
			compilation_status->append(vert_infolog);
			LOG_ERROR(compilation_status->c_str());
			return;
		}
		GFX_API::GFX_ShaderSource shadersource;
		shadersource.ASSET_ID = Asset_ID;
		shadersource.GL_ID = SHADER_o;
		SHADERSOURCEs.push_back(shadersource);
		compilation_status->append("Succesfully compiled!");
	}
	void GPU_ContentManager::Delete_ShaderSource(unsigned int Asset_ID) {
		GFX_API::GFX_ShaderSource* SHADER = nullptr;
		unsigned int vector_index = 0;
		SHADER = Find_GFXShaderSource_byID(Asset_ID, &vector_index);
		if (SHADER) {
			if (SHADER->GL_ID) {
				glDeleteShader(*(unsigned int*)SHADER->GL_ID);
				delete (unsigned int*)SHADER->GL_ID;
				SHADER->GL_ID = nullptr;
			}
			SHADER->GL_ID = nullptr;
			SHADERSOURCEs.erase(SHADERSOURCEs.begin() + vector_index);
			return;
		}
		LOG_WARNING("Unload has failed because intended Shader Source isn't found in OpenGL::GPU_ContentManager!");
	}
	void GPU_ContentManager::Compile_ComputeShader(GFX_API::ComputeShader_Resource* SHADER, unsigned int Asset_ID, string* compilation_status) {
		OGL4_ComputeShader* OGL_CS = new OGL4_ComputeShader;

		//Compile Compute Shader Object
		{
			OGL_CS->ComputeShader_ID = glCreateShader(GL_COMPUTE_SHADER);
			auto x = SHADER->SOURCE_CODE.c_str();
			glShaderSource(OGL_CS->ComputeShader_ID, 1, &x, NULL);
			glCompileShader(OGL_CS->ComputeShader_ID);

			//Check compile issues!
			int success;
			char vert_infolog[512];
			glGetShaderiv(OGL_CS->ComputeShader_ID, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(OGL_CS->ComputeShader_ID, 512, NULL, vert_infolog);
				compilation_status->append(vert_infolog);
				LOG_ERROR(compilation_status->c_str());
				return;
			}
		}

		//Link to a Shader Program
		{
			OGL_CS->ShaderProgram_ID = glCreateProgram();

			glAttachShader(OGL_CS->ShaderProgram_ID, OGL_CS->ComputeShader_ID);
			glLinkProgram(OGL_CS->ShaderProgram_ID);

			//Check linking issues
			int link_success;
			char link_infolog[512];
			glGetProgramiv(OGL_CS->ShaderProgram_ID, GL_LINK_STATUS, &link_success);
			if (!link_success) {
				glGetProgramInfoLog(OGL_CS->ShaderProgram_ID, 512, NULL, link_infolog);
				compilation_status->append(link_infolog);
				LOG_CRASHING(link_infolog);
				return;
			}

			//Bind Global Buffers
			Bind_GlobalBuffersto_ShaderProgram(SHADER->GLOBALBUFFERs, OGL_CS->ShaderProgram_ID);
		}

		GFX_API::GFX_ComputeShader shadersource;
		shadersource.ASSET_ID = Asset_ID;
		shadersource.GL_ID = OGL_CS;
		COMPUTESHADERs.push_back(shadersource);
		compilation_status->append("Succesfully compiled!");
	}
	void GPU_ContentManager::Delete_ComputeShader(unsigned int Asset_ID) {
		GFX_API::GFX_ComputeShader* SHADER = nullptr;
		unsigned int vector_index = 0;
		SHADER = Find_GFXComputeShader_byID(Asset_ID, &vector_index);
		if (SHADER) {
			if (SHADER->GL_ID) {
				OGL4_ComputeShader* OGL_CS = (OGL4_ComputeShader*)SHADER->GL_ID;
				glDeleteShader(OGL_CS->ComputeShader_ID);
				glDeleteProgram(OGL_CS->ShaderProgram_ID);
				delete OGL_CS;
			}
			SHADER->GL_ID = nullptr;
			SHADERSOURCEs.erase(SHADERSOURCEs.begin() + vector_index);
			return;
		}
		LOG_CRASHING("Unload has failed because intended Shader Source isn't found in OpenGL::GPU_ContentManager!");
	}
	void GPU_ContentManager::Link_MaterialType(GFX_API::Material_Type* MATTYPE_ASSET, unsigned int Asset_ID, string* compilation_status) {
		//Link and return the Shader Program!
		unsigned int* program_id = new unsigned int;
		*program_id = glCreateProgram();

		void* VS_ID = Find_GFXShaderSource_byID(MATTYPE_ASSET->VERTEXSOURCE_ID)->GL_ID;
		if (!VS_ID) {
			LOG_CRASHING("Vertex Shader isn't uploaded to GPU, so Shader Program linking failed!");
		}
		if (MATTYPE_ASSET->GEOMETRYSOURCE_ID != UINT32_MAX) {
			void* GS_ID = Find_GFXShaderSource_byID(MATTYPE_ASSET->GEOMETRYSOURCE_ID)->GL_ID;
			if (!GS_ID) {
				LOG_CRASHING("Geometry Shader isn't uploaded to GPU, so Shader Program linking failed!");
			}
			glAttachShader(*program_id, *(unsigned int*)GS_ID);
		}
		void* FS_ID = Find_GFXShaderSource_byID(MATTYPE_ASSET->FRAGMENTSOURCE_ID)->GL_ID;
		if (!FS_ID) {
			LOG_CRASHING("Vertex Shader isn't uploaded to GPU, so Shader Program linking failed!");
		}
		//Link Vertex and Fragment Shader to Shader Program and set ID
		glAttachShader(*program_id, *(unsigned int*)VS_ID);
		glAttachShader(*program_id, *(unsigned int*)FS_ID);
		glLinkProgram(*program_id);

		//Check linking issues
		int link_success;
		char link_infolog[512];
		glGetProgramiv(*program_id, GL_LINK_STATUS, &link_success);
		if (!link_success) {
			glGetProgramInfoLog(*program_id, 512, NULL, link_infolog);
			compilation_status->append(link_infolog);
			LOG_CRASHING(link_infolog);
			return;
		}

		//Bind Global Buffers
		Bind_GlobalBuffersto_ShaderProgram(MATTYPE_ASSET->GLOBALBUFFERs, *program_id);

		GFX_API::GFX_ShaderProgram SHADERPROGRAM;
		SHADERPROGRAM.ASSET_ID = Asset_ID;
		SHADERPROGRAM.GL_ID = program_id;
		SHADERPROGRAMs.push_back(SHADERPROGRAM);
		compilation_status->append("Succesfully linked!");
	}
	void GPU_ContentManager::Delete_MaterialType(unsigned int Asset_ID) {
		GFX_API::GFX_ShaderProgram* PROGRAM = nullptr;
		unsigned int vector_index = 0;
		PROGRAM = Find_GFXShaderProgram_byID(Asset_ID, &vector_index);
		if (PROGRAM) {
			if (PROGRAM->GL_ID) {
				glDeleteProgram(*(unsigned int*)PROGRAM->GL_ID);
				delete (unsigned int*)PROGRAM->GL_ID;
				PROGRAM->GL_ID = nullptr;
			}
			PROGRAM->GL_ID = nullptr;
			SHADERPROGRAMs.erase(SHADERPROGRAMs.begin() + vector_index);
			return;
		}
		LOG_WARNING("Unload has failed because intended Material Type isn't found in OpenGL::GPU_ContentManager!");
	}

	unsigned int GPU_ContentManager::Create_Framebuffer() {
		GFX_API::Framebuffer FB;
		
		FB.GL_ID = new unsigned int(0);
		glGenFramebuffers(1, (unsigned int*)FB.GL_ID);
		GFX->Check_Errors();
		FB.ID = Create_FrameBufferID();
		FBs.push_back(FB);
		return FB.ID;
	}
	void GPU_ContentManager::Delete_Framebuffer(unsigned int Framebuffer_ID) {
		GFX_API::Framebuffer* FB = nullptr;
		unsigned int vector_index = 0;
		FB = Find_Framebuffer_byGFXID(Framebuffer_ID, &vector_index);
		if (FB) {
			if (FB->GL_ID) {
				glDeleteFramebuffers(1, (unsigned int*)FB->GL_ID);
				delete (unsigned int*)FB->GL_ID;
			}
			FB->GL_ID = nullptr;
			FBs.erase(FBs.begin() + vector_index);
			return;
		}
		LOG_WARNING("Unload has failed because intended Framebuffer isn't found in OpenGL::GPU_ContentManager!");
	}
	void GPU_ContentManager::Attach_RenderTarget_toFramebuffer(const GFX_API::Framebuffer::RT_SLOT* RT_SLOT, GFX_API::RT_ATTACHMENTs ATTACHMENT_TYPE, unsigned int FB_ID) {
		GFX_API::Framebuffer* FB = nullptr;
		FB = Find_Framebuffer_byGFXID(FB_ID);

		GFX_API::GFX_Texture* RT = nullptr;
		RT = Find_GFXTexture_byID(RT_SLOT->RT_ID);
		unsigned int RT_GLID = *(unsigned int*)RT->GL_ID;

		if (RT_GLID) {
			glBindFramebuffer(GL_FRAMEBUFFER, *(unsigned int*)Find_Framebuffer_byGFXID(FB_ID)->GL_ID);
			glBindTexture(GL_TEXTURE_2D, RT_GLID);
			glFramebufferTexture2D(GL_FRAMEBUFFER, Find_RenderTarget_AttachmentType(ATTACHMENT_TYPE), GL_TEXTURE_2D, RT_GLID, 0);
		}
		else {
			LOG_CRASHING("Creating a Render Buffer for RT isn't supported for now, so you can't bind it to Framebuffer! There is a problem somewhere");
			return;
		}
		Check_ActiveFramebuffer_Status(std::to_string(FB->ID).c_str());


		FB->BOUND_RTs.push_back(*RT_SLOT);
	}
	void* GPU_ContentManager::ReadFramebufferAttachment(GFX_API::RT_ATTACHMENTs ATTACHMENTTYPE, unsigned int FB_ID, unsigned int& DATASIZE) {
		GLint boundfb = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &boundfb);
		GFX_API::Framebuffer* fb = nullptr;
		fb = Find_Framebuffer_byGFXID(FB_ID);
		glBindFramebuffer(GL_FRAMEBUFFER, *(unsigned int*)fb->GL_ID);
		GLenum en = 0; unsigned int rt_compsize = 0;
		switch (ATTACHMENTTYPE) {
		case GFX_API::RT_ATTACHMENTs::TEXTURE_ATTACHMENT_COLOR0: LOG_CRASHING("Color attachment isn't supported properly!"); en = GL_RGBA; rt_compsize = 16; break;
		case GFX_API::RT_ATTACHMENTs::TEXTURE_ATTACHMENT_DEPTH: en = GL_DEPTH_COMPONENT; rt_compsize = 4; break;
		case GFX_API::RT_ATTACHMENTs::TEXTURE_ATTACHMENT_DEPTHSTENCIL: en = GL_DEPTH_STENCIL; rt_compsize = 4; break;
		}
		GFXContentManager->Find_GFXTexture_byID(fb->BOUND_RTs[0].RT_ID)->ASSET_ID;
		DATASIZE = fb->BOUND_RTs[0].WIDTH * fb->BOUND_RTs[0].HEIGHT * rt_compsize;
		void* returneddata = new char[DATASIZE];
		glReadPixels(0, 0, fb->BOUND_RTs[0].WIDTH, fb->BOUND_RTs[0].HEIGHT, en, GL_FLOAT, returneddata);

		glBindFramebuffer(GL_FRAMEBUFFER, boundfb);

		return returneddata;
	}

	unsigned int GPU_ContentManager::Create_BindingPoint() {
		//We can use the Binding Point 0!
		unsigned int ID = UNIFORMBUFFER_BINDINGPOINTs.GetIndex_FirstFalse();
		UNIFORMBUFFER_BINDINGPOINTs.SetBit_True(ID);
		return ID;
	}
	void GPU_ContentManager::Delete_BindingPoint(unsigned int ID) {
		UNIFORMBUFFER_BINDINGPOINTs.SetBit_False(ID);
	}
}