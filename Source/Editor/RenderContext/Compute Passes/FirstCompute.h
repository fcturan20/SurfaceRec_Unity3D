#pragma once
#include "Editor/Editor_Includes.h"
#include "GFX/GFX_Core.h"


namespace TuranEditor {

	class FirstCompute : public GFX_API::ComputePass {
		static unsigned int RAYTRACECAMERABUFFER_ID;
		static void* RAYTRACECAMERABUFFER_DATA;
	public:
		static vec3 RAYTRACECAMERA_POS, RAYTRACECAMERA_FRONTVECTOR;
		static unsigned int OUTPUTTEXTURE_ID, CS_ID, CSinst_ID;
		FirstCompute();
		virtual void Execute() override;
	};


}