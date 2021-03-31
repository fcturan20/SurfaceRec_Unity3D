#pragma once
#include "GFX/GFX_Includes.h"
#include "GFX_RenderCommands.h"
#include "GFX_Resource.h"
#include "GFX/GFX_FileSystem/Resource_Type/Material_Type_Resource.h"
#include "GFX/GFX_ENUMs.h"

namespace GFX_API {
	class GFXAPI RenderNode {
		RENDERNODE_TYPEs RenderNode_Type;
	};


	//If you want to create a Draw Pass:
	//1) You should create a class inherited from this
	//2) Include the class header in GFX API specific header (For example: OGL3_Core)
	//3) Create an object as "new" in GFX API specfic class's Start_Creation()
	//4) Don't forget to give a name to the draw pass! It will show up in Framegraph, Debugging and Profiling!
	class GFXAPI DrawPass : public RenderNode {
	protected:
		//These vectors store indexes to the RenderGraph's vectors!
		vector<unsigned int> DrawCallBuffer;
		const vector<DrawCall>& RG_DrawCallBuffer;
		const vector<PointLineDrawCall>& RG_PointDrawCallBuffer;

		unsigned int FRAMEBUFFER;
		bool Is_SetupPhase_Called;
		string NAME;
	public:
		DrawPass(const vector<DrawCall>& i_RG_DrawCallBuffer, const vector<PointLineDrawCall>& i_RG_PointDrawCallBuffer, const char* NAME);

		//While constructing a RenderGraph, call this to couple DrawPass and RenderGraph
		//So, DrawCallBuffer should point RenderGraph's DrawCalls vector!
		virtual void RenderGraph_SetupPhase(vector<GFX_API::Framebuffer::RT_SLOT>& RTs) = 0;
		//Update DrawCallBuffer!
		virtual void ResourceUpdatePhase() = 0;
		virtual void Execute() = 0;

		//GETTER-SETTERs

		const char* Get_Name();
		unsigned int Get_FramebufferID();
	};

	class GFXAPI ComputePass : public RenderNode {
	protected:
		vector<ComputeShader_Instance*> ComputeShaders;
	public:
		virtual void Execute() = 0;
	};


	class GFXAPI RenderGraph {
	protected:
		vector<RenderNode*> RENDER_NODEs;
		vector<DrawCall> DrawCalls;
		vector<PointLineDrawCall> PointDrawCallBuffer;
		unsigned int RenderGraph_ID;

	public:
		string NAME;
		RenderGraph(const char* name);

		void Register_DrawCall(DrawCall drawcall);
		void Register_PointDrawCall(PointLineDrawCall pointdrawcall);
		//Don't call this function, this function is only called by GFX APIs
		virtual void Run_RenderGraph() = 0;
		const vector<const RenderNode*> Get_RenderNodes();

		//GETTER-SETTERs

		void Set_RenderGraphID(unsigned int ID);
		unsigned int Get_RenderGraph_ID() const;
	};
}