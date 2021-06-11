#include "GPU_ContentManager.h"

namespace GFX_API {
	VertexAttribute& VertexAttribute::operator=(const VertexAttribute& attribute) {
		AttributeName = attribute.AttributeName;
		DATATYPE = attribute.DATATYPE;
		Index = attribute.Index;
		Stride = attribute.Stride;
		Start_Offset = attribute.Start_Offset;
		return *this;
	}

	VertexAttributeLayout::VertexAttributeLayout() {

	}
	VertexAttributeLayout& VertexAttributeLayout::operator=(const VertexAttributeLayout& layout) {
		Attributes = layout.Attributes;
		size_pervertex = layout.size_pervertex;
		return *this;
	}
	void VertexAttributeLayout::Calculate_SizeperVertex() {
		size_pervertex = 0;
		for (unsigned int i = 0; i < Attributes.size(); i++) {
			VertexAttribute& Attribute = Attributes[i];
			size_pervertex += Get_UNIFORMTYPEs_SIZEinbytes(Attribute.DATATYPE);
		}
	}

	bool VertexAttributeLayout::VerifyAttributeLayout() const {
		int FirstContinuousAttributeIndex = -1;
		for (unsigned int attribute_index = 0; attribute_index < Attributes.size(); attribute_index++) {
			const VertexAttribute& current_attribute = Attributes[attribute_index];


			//Wrong Index
			if (current_attribute.Index != attribute_index) {
				string Error_Message = "Wrong Index in Attribute: ";
				Error_Message.append(current_attribute.AttributeName);
				std::cout << "Attribute's index: " << current_attribute.Index << " but should be: " << attribute_index << std::endl;
				LOG_ERROR(Error_Message);
				return false;
			}

			//Find the first Continuous Attribute and check its Start_Offset
			if (FirstContinuousAttributeIndex == -1 && current_attribute.Stride == 0 && attribute_index > 0) {
				//First Continuous Attribute has to have a Start_Offset, because previous data is Interleaved!
				if (current_attribute.Start_Offset == 0) {
					LOG_CRASHING("Please read the specification about vertex Attribute Layout! It's not allowed to have a interleaved attribute after an continuous attribute!");
					return false;
				}
				FirstContinuousAttributeIndex = attribute_index;
			}

			//Layout starts with Continuous Attribute, there shouldn't be any Interleaved Attribute!
			else if (FirstContinuousAttributeIndex == -1 && current_attribute.Stride == 0) {
				FirstContinuousAttributeIndex = attribute_index;
			}

			if (attribute_index > 0) {
				const VertexAttribute& previous_attribute = Attributes[attribute_index - 1];

				//Start Offset = 0 shouldn't be used if previous attribute isn't continous!
				if (current_attribute.Start_Offset == 0 && previous_attribute.Stride != 0) {
					LOG_CRASHING("Please read the specification about Vertex Attribute Layout! Start_Offset = 0 shouldn't be used if previous attribute isn't continuous!");
					return false;
				}

				//It's not allowed to have a interleaved attribute after an continuous attribute!
				else if (current_attribute.Stride != 0 && previous_attribute.Stride == 0) {
					LOG_CRASHING("Please read the specification about vertex Attribute Layout! It's not allowed to have a interleaved attribute after an continuous attribute!");
					return false;
				}
			}

			return true;
		}
	}


	char VertexAttributeLayout::Find_AttributeIndex_byName(const char* Attribute_Name) const {
		for (unsigned char i = 0; i < Attributes.size(); i++) {
			if (Attribute_Name == Attributes[i].AttributeName) {
				return i;
			}
		}
		
		//Failed to find!
		LOG_CRASHING("Failed to find Vertex Attribute!");
		return -1;
	}
	void* VertexAttributeLayout::Gather_AttributeData(const void* vertex_data, size_t datasize_inbytes, size_t vertex_count, unsigned char Attribute_Index, size_t& data_size) const {
		return nullptr;
	}





	GPU_ContentManager::GPU_ContentManager() : MESHID_BITSET(1000), POINTBUFFERID_BITSET(1000), BUFFERID_BITSET(100), FBID_BITSET(30){

	}

	GPU_ContentManager::~GPU_ContentManager() {
		std::cout << "GFX's GPU Content Manager's destructor is called!\n";
	}

	//ID OPERATIONs

	unsigned int GPU_ContentManager::Create_MeshBufferID() {
		unsigned int ID = MESHID_BITSET.GetIndex_FirstFalse() + 1;
		MESHID_BITSET.SetBit_True(ID - 1);
		return ID;
	}
	void GPU_ContentManager::Delete_MeshBufferID(unsigned int ID) {
		MESHID_BITSET.SetBit_False(ID - 1);
	}
	unsigned int GPU_ContentManager::Create_PointBufferID() {
		unsigned int ID = POINTBUFFERID_BITSET.GetIndex_FirstFalse() + 1;
		POINTBUFFERID_BITSET.SetBit_True(ID - 1);
		return ID;
	}
	void GPU_ContentManager::Delete_PointBufferID(unsigned int ID) {
		POINTBUFFERID_BITSET.SetBit_False(ID - 1);
	}
	unsigned int GPU_ContentManager::Create_GlobalBufferID() {
		unsigned int ID = BUFFERID_BITSET.GetIndex_FirstFalse() + 1;
		BUFFERID_BITSET.SetBit_True(ID - 1);
		return ID;
	}
	void GPU_ContentManager::Delete_GlobalBufferID(unsigned int ID) {
		BUFFERID_BITSET.SetBit_False(ID - 1);
	}
	unsigned int GPU_ContentManager::Create_FrameBufferID() {
		unsigned int ID = FBID_BITSET.GetIndex_FirstFalse() + 1;
		FBID_BITSET.SetBit_True(ID - 1);
		return ID;
	}
	void GPU_ContentManager::Delete_FrameBufferID(unsigned int ID) {
		FBID_BITSET.SetBit_False(ID - 1);
	}

	//HELPER FUNCTIONs

	GFX_Mesh* GPU_ContentManager::Find_MeshBuffer_byBufferID(unsigned int MeshBufferID, unsigned int* vector_index) {
		for (unsigned int i = 0; i < MESHBUFFERs.size(); i++) {
			if (MESHBUFFERs[i].BUFFER_ID == MeshBufferID) {
				if (vector_index) {
					*vector_index = i;
				}
				return &MESHBUFFERs[i];
			}
		}
		LOG_WARNING("Intended Mesh Buffer isn't found in GPU_ContentManager!");
	}
	GFX_Point* GPU_ContentManager::Find_PointBuffer_byBufferID(unsigned int PointBufferID, unsigned int* vector_index) {
		for (unsigned int i = 0; i < POINTBUFFERs.size(); i++) {
			if (POINTBUFFERs[i].BUFFER_ID == PointBufferID) {
				if (vector_index) {
					*vector_index = i;
				}
				return &POINTBUFFERs[i];
			}
		}
		LOG_WARNING("Intended Point Buffer isn't found in GPU_ContentManager!");
		return nullptr;
	}
	GFX_Buffer* GPU_ContentManager::Find_GlobalBuffer_byBufferID(unsigned int GlobalBufferID, unsigned int* vector_index) {
		for (unsigned int i = 0; i < GLOBALBUFFERs.size(); i++) {
			if (GLOBALBUFFERs[i].ID == GlobalBufferID) {
				if (vector_index) {
					*vector_index = i;
				}
				return &GLOBALBUFFERs[i];
			}
		}
		LOG_WARNING("Intended Global Buffer isn't found in GPU_ContentManager!");
		return nullptr;
	}
	GFX_API::Framebuffer* GPU_ContentManager::Find_Framebuffer_byGFXID(unsigned int FB_ID, unsigned int* vector_index) {
		for (unsigned int i = 0; i < FBs.size(); i++) {
			if (FB_ID == FBs[i].ID) {
				if (vector_index) {
					*vector_index = i;
				}
				return &FBs[i];
			}
		}
		LOG_WARNING("Intended Framebuffer isn't found in GPU_ContentManager!");
	}
	GFX_Texture* GPU_ContentManager::Find_GFXTexture_byID(unsigned int Texture_AssetID, unsigned int* vector_index) {
		for (unsigned int i = 0; i < TEXTUREs.size(); i++) {
			GFX_API::GFX_Texture& TEXTURE = TEXTUREs[i];
			if (TEXTURE.ASSET_ID == Texture_AssetID) {
				if (vector_index) {
					*vector_index = i;
				}
				return &TEXTURE;
			}
		}
		LOG_WARNING("Intended Texture isn't uploaded to GPU!" + to_string(Texture_AssetID));
		return nullptr;
	}
	GFX_ShaderSource* GPU_ContentManager::Find_GFXShaderSource_byID(unsigned int ShaderSource_AssetID, unsigned int* vector_index) {
		for (unsigned int i = 0; i < SHADERSOURCEs.size(); i++) {
			GFX_API::GFX_ShaderSource& SHADERSOURCE = SHADERSOURCEs[i];
			if (SHADERSOURCE.ASSET_ID == ShaderSource_AssetID) {
				if (vector_index) {
					*vector_index = i;
				}
				return &SHADERSOURCE;
			}
		}
		LOG_WARNING("Intended ShaderSource isn't uploaded to GPU!");
	}
	GFX_ComputeShader* GPU_ContentManager::Find_GFXComputeShader_byID(unsigned int ComputeShader_AssetID, unsigned int* vector_index) {
		for (unsigned int i = 0; i < COMPUTESHADERs.size(); i++) {
			GFX_API::GFX_ComputeShader& SHADERSOURCE = COMPUTESHADERs[i];
			if (SHADERSOURCE.ASSET_ID == ComputeShader_AssetID) {
				if (vector_index) {
					*vector_index = i;
				}
				return &SHADERSOURCE;
			}
		}
		LOG_WARNING("Intended ComputeShader isn't uploaded to GPU!");
	}
	GFX_ShaderProgram* GPU_ContentManager::Find_GFXShaderProgram_byID(unsigned int ShaderProgram_AssetID, unsigned int* vector_index) {
		for (unsigned int i = 0; i < SHADERPROGRAMs.size(); i++) {
			GFX_API::GFX_ShaderProgram& SHADERPROGRAM = SHADERPROGRAMs[i];
			if (SHADERPROGRAM.ASSET_ID == ShaderProgram_AssetID) {
				if (vector_index) {
					*vector_index = i;
				}
				return &SHADERPROGRAM;
			}
		}
		LOG_CRASHING("Intended ShaderProgram isn't uploaded to GPU!");
		return nullptr;
	}
}