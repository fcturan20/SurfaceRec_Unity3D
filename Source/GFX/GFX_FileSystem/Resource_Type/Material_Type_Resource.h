#pragma once
#include "GFX/GFX_Includes.h"
#include "GFX/GFX_ENUMs.h"

enum class RESOURCETYPEs : char;

namespace GFX_API {
	struct GFXAPI Material_Uniform {
		string VARIABLE_NAME;
		DATA_TYPE VARIABLE_TYPE;
		void* DATA = nullptr;
		Material_Uniform(const char* variable_name, DATA_TYPE variable_type);
		bool Verify_UniformData();
		Material_Uniform();
	};


	//This class doesn't support Compute, only the Rasterization stages
	class GFXAPI ShaderSource_Resource {
	public:
		ShaderSource_Resource();
		SHADER_STAGE STAGE;
		string SOURCE_CODE;
		SHADER_LANGUAGEs LANGUAGE;
	};
	
	/*
		1) ACCESS_TYPE defines the texture's binding way
		2) OP_TYPE defines why shader accesses the texture. 
	If a texture is READ_WRITE but you just read it in the shader, you can set as READ_ONLY
		3) TEXTURE_ID is texture's asset ID. But render targets (framebuffer attachments) aren't assets.
		So, TEXTURE_ID isn't important if ACCESS_TYPE is FRAMEBUFFER_ATTACHMENT
	*/
	struct GFXAPI Texture_Access {
		TEXTURE_DIMENSIONs DIMENSIONs;
		TEXTURE_CHANNELs CHANNELs;
		OPERATION_TYPE OP_TYPE;
		TEXTURE_ACCESS ACCESS_TYPE;
		unsigned int BINDING_POINT;		
		unsigned int TEXTURE_ID;
	};

	
	//ACCESS_TYPE isn't used for anything, just an additional information
	struct GFXAPI GlobalBuffer_Access {
		unsigned int BUFFER_ID;
		OPERATION_TYPE ACCESS_TYPE;
	};

	class GFXAPI Material_Type {
	public:
		Material_Type();

		unsigned int VERTEXSOURCE_ID, FRAGMENTSOURCE_ID;

		vector<Texture_Access> TEXTUREs;
		vector<Material_Uniform> UNIFORMs;
		vector<GlobalBuffer_Access> GLOBALBUFFERs;
	};
	
	//Don't forget, data handling for each uniform type is the responsibility of the user!
	class GFXAPI Material_Instance {
	public:
		Material_Instance();
		
		//Uniforms won't change at run-time because we are defining uniforms at compile-time, but it is an easier syntax for now!
		//This list will be defined per material type (for example: Surface_PBR, Surface_Phong, Texture View etc.)
		unsigned int Material_Type;
		vector<Texture_Access> TEXTURE_LIST;
		vector<Material_Uniform> UNIFORM_LIST;

		unsigned int Find_Uniform_byName(const char* uniform_name);
		void Set_Uniform_Data(const char* uniform_name, void* pointer_to_data);
	};



	class GFXAPI ComputeShader_Resource {
	public:
		SHADER_LANGUAGEs LANGUAGE;
		string SOURCE_CODE;
		vector<Texture_Access> TEXTUREs;
		vector<Material_Uniform> UNIFORMs;
		vector<GlobalBuffer_Access> GLOBALBUFFERs;
	};

	class GFXAPI ComputeShader_Instance {
	public:
		unsigned int ComputeShader;
		vector<Texture_Access> TEXTURE_LIST;
		vector<Material_Uniform> UNIFORM_LIST;
	};
}
