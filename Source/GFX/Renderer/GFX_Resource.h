#pragma once
#include "GFX/GFX_Includes.h"
#include "GFX/GFX_FileSystem/Resource_Type/Texture_Resource.h"

/*
	All resources have GL_ID variable. This variable is a pointer to GL specific structure that's used by GL functions (For example; unsigned ints for Textures, Buffers etc. in OpenGL)
	
*/



namespace GFX_API {
	/* Vertex Attribute Layout Specification!
	If Stride = 0, attribute data is continous in memory.
	If Start_Offset is 0, that means previous attribute is contiunous and this attributes starts where previous attribute ends
	If you want previous attribute is interleaved, but this attribute is continous; you should set Start_Offset by yourself, Start_Offset = 0 will cause undefined behaviour.
	Buffer Layout have to be: Interleaved attributes first, Continuous attributes later. You can't have an interleaved attribute data after an continous attribute or mix like Interleaved - Continuous - Interleaved
	
				EXAMPLES: Position(vec3) Attribute Data = 1, TextCoord(vec2) Data = 2, VertexColor(vec3) Data = 3, VertexNormal(vec3) Data = 4
	1) Memory: 1,1,1,2,2,2,3,3,3,4,4,4
	All attributes -> Stride = 0, Start_Offset = 0
	2) Memory: 1,2,1,2,1,2,3,3,3,4,4,4
	Position Attributes -> Stride = sizeof(vec3) + sizeof(vec2), Start_Offset = 0; TextCoord Attribute-> Stride = sizeof(vec3) + sizeof(vec2), Start_Offset = sizeof(vec3); VertexColor Attribute -> Stride = 0, Start_Offset = (sizeof(vec3) + sizeof(vec2)) * vertex_count;
		VertexNormal Attribute -> Stride = 0, Start_Offset = 0
	3) Memory: 1,1,1,2,3,4,2,3,4,2,3,4
	This is not allowed; always Interleaved attributes first, then Continuous!
	Note: In example 2, if you set VertexColor's Start_Offset to 0; that would cause undefined behaviour because previous attribute (TextCoord) isn't contiunous.
	With vertex count, I could find where to start the continuous attribute even if the previous attribute is interleaved but this requires so many code and costs some performance
	because we need all the previous attributes and possibilities rises permutational (nested loops needed).
	Note: Both example 2 and 3 should be created for each mesh, because they need vertex count!
	*/
	struct GFXAPI VertexAttribute {
		string AttributeName;
		DATA_TYPE DATATYPE;
		unsigned char Index;
		size_t Stride;
		size_t Start_Offset;
		VertexAttribute& operator=(const VertexAttribute& attribute);
	};

	struct GFXAPI VertexAttributeLayout {
		VertexAttributeLayout();
		VertexAttributeLayout& operator=(const VertexAttributeLayout& layout);
		vector<VertexAttribute> Attributes;
		//All of the attributes are per vertex, so this is the size of attribute data per vertex
		unsigned int size_pervertex;
		void Calculate_SizeperVertex();
		bool VerifyAttributeLayout() const;
		//Fail value is -1
		char Find_AttributeIndex_byName(const char* Attribute_Name) const;
		//Gathers all data of the specified attribute somewhere in the memory!
		//Return pointer is the pointer of gathered data and last argument data_size is the size of the data!
		//If you are not sure whether the vertex data fits your attribute layout, you should use Does_BufferFits_Layout() first!
		//If you don't know your attribute index, you can use Find
		void* Gather_AttributeData(const void* vertex_data, size_t datasize_inbytes, size_t vertex_count, unsigned char Attribute_Index, size_t& data_size) const;
	};



	struct GFXAPI Framebuffer {
		struct RT_SLOT {
			unsigned int RT_ID, WIDTH, HEIGHT;
			RT_ATTACHMENTs ATTACHMENT_TYPE;
			OPERATION_TYPE RT_OPERATIONTYPE;
			RT_READSTATE RT_READTYPE;
			vec3 CLEAR_COLOR;
		};
	public:
		unsigned int ID;
		void* GL_ID;
		vector<RT_SLOT> BOUND_RTs;

		Framebuffer();
	};

	struct GFXAPI GFX_Mesh {
		unsigned int BUFFER_ID, VERTEX_COUNT, INDEX_COUNT;
		void* GL_ID;
	};

	//There is a one-way relationship between GFX_Point and GFX_Mesh
	//If Point Buffer uses first vertex attribute of Mesh Buffer, point buffer uses the GFX_Mesh's GL_ID to point at the buffer in GPU-side.
	//That means if you unload the GFX_Mesh that Point Buffer uses its first vertex attribute, you shouldn't draw GFX_Point. Otherwise undefined behaviour.
	//If you use any other vertex attribute of GFX_Mesh, we re-upload the data to GPU. That means even if you delete GFX_Mesh, you can draw GFX_Point without any issue.
	struct GFXAPI GFX_Point {
		unsigned int BUFFER_ID, POINT_COUNT;
		GFX_API::VertexAttributeLayout PointAttributeLayout;
		void* GL_ID;
	};

	struct GFXAPI GFX_Texture {
		unsigned int ASSET_ID;
		void* GL_ID;
	};

	/*
		GLSL uses shader name reflection to get Buffer, so NAME should be the same thing as in the shader
		Also your global buffers in shaders shouldn't specify binding point, buffer's name should be the NAME here
	*/
	struct GFXAPI GFX_Buffer {
		string NAME;		
		unsigned int ID;
		const void* DATA,
			*GL_ID,			//Buffer's ID given by GL
			*BINDING_POINT; //Shaders uses binding point to access the buffer, so store it.
		unsigned int DATA_SIZE;
		BUFFER_VISIBILITY USAGE;
	};




	//This represents Rasterization Shader Stages, not Compute
	struct GFXAPI GFX_ShaderSource {
		unsigned int ASSET_ID;
		void* GL_ID;
	};

	struct GFXAPI GFX_ShaderProgram {
		unsigned int ASSET_ID;
		void* GL_ID;
	};

	struct GFXAPI GFX_ComputeShader {
		unsigned int ASSET_ID;
		void* GL_ID;
	};

	struct GFXAPI RenderState {
		DEPTH_MODEs DEPTH_MODE;
		DEPTH_TESTs DEPTH_TEST_FUNC;
		//There should be Stencil and Blending options, but for now leave it like this
	};
}