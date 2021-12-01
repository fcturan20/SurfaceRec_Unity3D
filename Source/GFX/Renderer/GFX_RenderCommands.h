#pragma once
#include "GFX/GFX_Includes.h"
#include "GFX/GFX_ENUMs.h"
#include "GFX_Resource.h"
#include "GFX/GFX_FileSystem/Resource_Type/Material_Type_Resource.h"

namespace GFX_API {
	/*
	1) This class will be used to store necessary informations to draw triangles
	*/
	struct GFXAPI DrawCall {
		unsigned int JoinedDrawPasses;	//This is a mask for each Draw Pass! 
		//Each Draw Pass will check this first, then ShaderInstance to check if this Draw Call suits its support range.
		//Unsigned int is 32 bit, so the maximum number of draw passes in a RenderGraph is 32 bit
		unsigned int MeshBuffer_ID;
		unsigned int ShaderInstance_ID;
	};

	/*
	Definitions of Points and Lines Rendering should be in the related Draw Pass
	That means, you aren't allowed to pass shader instance and you can't specify colors or sizes of lines and points outside of shader
	I want to support Point and Line rendering for debugging purposes, nothing more
	*/
	struct GFXAPI PointLineDrawCall {
		bool Draw_asPoint;	//True -> Draw as Point, False -> Draw as Line
		unsigned int ShaderInstance_ID;
		unsigned int PointBuffer_ID;
	};


	struct GFXAPI SpecialDrawCall {
		unsigned int ShaderInstance_ID, VERTEXCOUNT;
		std::vector<Material_Uniform> OverridenUniforms;
		//If you are gonna render triangles as lines, this should be true.
		//If you are gonna render triangles filled, this should be false.
		bool isLine;
	};
}