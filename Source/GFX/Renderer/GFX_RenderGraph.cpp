#include "GFX_RenderGraph.h"
#include "TuranAPI/Logger_Core.h"

namespace GFX_API {
	//Getter-Setters
	DrawPass::DrawPass(const vector<DrawCall>& i_RG_DrawCallBuffer_, const vector<PointLineDrawCall>& i_RG_PointDrawCallBuffer, const vector<SpecialDrawCall>& i_RGSpecialDrawCallBuffer, const char* name): NAME(name),
		Is_SetupPhase_Called(false)
		, RG_DrawCallBuffer(i_RG_DrawCallBuffer_), RG_PointDrawCallBuffer(i_RG_PointDrawCallBuffer), RG_SpecialDrawCalls(i_RGSpecialDrawCallBuffer){

	}

	const char* DrawPass::Get_Name() {
		return NAME.c_str();
	}

	unsigned int DrawPass::Get_FramebufferID() {
		return FRAMEBUFFER;
	}


	RenderGraph::RenderGraph(const char* name) : NAME(name){
		LOG_STATUS("Render Graph object is created!");
	}

	void RenderGraph::Set_RenderGraphID(unsigned int ID) {
		RenderGraph_ID = ID;
		//LOG_STATUS("RenderGraph: " + NAME + "'s ID is set as " + to_string(ID));
	}
	unsigned int RenderGraph::Get_RenderGraph_ID() const {
		return RenderGraph_ID;
	}
	void RenderGraph::Register_DrawCall(DrawCall drawcall) {
		DrawCalls.push_back(drawcall);
	}
	void RenderGraph::Register_PointDrawCall(PointLineDrawCall drawcall) {
		PointDrawCallBuffer.push_back(drawcall);
	}
	void RenderGraph::Register_SpecialDrawCall(SpecialDrawCall specialdrawcall) {
		SpecialDrawCallBuffer.push_back(specialdrawcall);
	}
	const vector<const RenderNode*> RenderGraph::Get_RenderNodes() {
		vector<const RenderNode*> RNs;
		for (RenderNode* RN : RENDER_NODEs) {
			RNs.push_back(RN);
		}
		return RNs;
	}
}