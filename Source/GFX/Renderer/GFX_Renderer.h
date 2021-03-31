#pragma once
#include "GFX/GFX_Includes.h"
#include "GFX_Resource.h"
#include "GFX_RenderCommands.h"
#include "GFX_RenderGraph.h"
#include "GFX/GFX_FileSystem/Resource_Type/Material_Type_Resource.h"

namespace GFX_API {
	class GFXAPI Renderer {
	protected:
		float LINEWIDTH = 1;
	public:
		friend class GFX_Core;

		Renderer();
		~Renderer();
		Renderer* SELF;


		//RENDERER FUNCTIONs
		virtual void Bind_Framebuffer(const Framebuffer* FB) = 0;
		virtual void Bind_MatInstance(Material_Instance* MATINST) = 0;
		virtual void Set_DepthTest(DEPTH_MODEs MODE, DEPTH_TESTs TEST) = 0;
		virtual void Set_CullingMode(CULL_MODE MODE) = 0;
		virtual void Set_LineWidth(float WIDTH) = 0;
		virtual void DrawTriangle(const GFX_Mesh* MeshBuffer) = 0;
		virtual void DrawPoint(const GFX_Point* PointBuffer) = 0;
		virtual void DrawLine(const GFX_Point* PointBuffer) = 0;
		virtual void Compute_Dispatch(const GFX_API::ComputeShader_Instance* CS, vec3 Dispatch_Groups) = 0;
	};
}