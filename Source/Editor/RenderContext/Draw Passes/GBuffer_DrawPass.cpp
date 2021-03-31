#include "GBuffer_DrawPass.h"
#include "GFX/GFX_Core.h"
#include "Editor/FileSystem/EditorFileSystem_Core.h"
#include "Editor/RenderContext/Editor_DataManager.h"
#include "Editor/FileSystem/ResourceTypes/Resource_Identifier.h"
#include "Editor/FileSystem/ResourceTypes/ResourceTYPEs.h"

#include <string>

unsigned int Main_DrawPass::Get_BitMaskFlag() {
	return 1;
}

Main_DrawPass::Main_DrawPass(const vector<GFX_API::DrawCall>& i_RG_DrawCallBuffer_, const vector<GFX_API::PointLineDrawCall>& i_RG_PointDrawCallBuffer, vector<GFX_API::Framebuffer::RT_SLOT>& Needed_RTSlots)
	: DrawPass(i_RG_DrawCallBuffer_, i_RG_PointDrawCallBuffer, "Main Pass") {
	GFX_API::Framebuffer::RT_SLOT Color0_SLOT;
	Color0_SLOT.ATTACHMENT_TYPE = GFX_API::RT_ATTACHMENTs::TEXTURE_ATTACHMENT_COLOR0;
	Color0_SLOT.RT_OPERATIONTYPE = GFX_API::OPERATION_TYPE::WRITE_ONLY;
	Color0_SLOT.RT_READTYPE = GFX_API::RT_READSTATE::CLEAR;
	Color0_SLOT.CLEAR_COLOR = vec3(0.5f, 0.7f, 0.7f);
	//These should be set by RenderGraph!
	Color0_SLOT.HEIGTH = 0; Color0_SLOT.WIDTH = 0; Color0_SLOT.RT_ID = 0;
	Needed_RTSlots.push_back(Color0_SLOT);
	

	GFX_API::Framebuffer::RT_SLOT Depth_SLOT;
	Depth_SLOT.ATTACHMENT_TYPE = GFX_API::RT_ATTACHMENTs::TEXTURE_ATTACHMENT_DEPTH;
	Depth_SLOT.RT_OPERATIONTYPE = GFX_API::OPERATION_TYPE::WRITE_ONLY;
	Depth_SLOT.RT_READTYPE = GFX_API::RT_READSTATE::CLEAR;
	Depth_SLOT.CLEAR_COLOR = vec3(1);						//Reversed Depth
	//These should be set by RenderGraph!
	Depth_SLOT.HEIGTH = 0; Depth_SLOT.WIDTH = 0; Depth_SLOT.RT_ID = 0;
	Needed_RTSlots.push_back(Depth_SLOT);

	Create_LineRendererMatInst();
}

void Main_DrawPass::Create_LineRendererMatInst() {
}

void Main_DrawPass::RenderGraph_SetupPhase(vector<GFX_API::Framebuffer::RT_SLOT>& RTs) {
	Is_SetupPhase_Called = true;

	FRAMEBUFFER = GFXContentManager->Create_Framebuffer();
	FB = GFXContentManager->Find_Framebuffer_byGFXID(FRAMEBUFFER);

	GFXContentManager->Attach_RenderTarget_toFramebuffer(&RTs[0], GFX_API::RT_ATTACHMENTs::TEXTURE_ATTACHMENT_COLOR0, FRAMEBUFFER);
	GFXContentManager->Attach_RenderTarget_toFramebuffer(&RTs[1], GFX_API::RT_ATTACHMENTs::TEXTURE_ATTACHMENT_DEPTH, FRAMEBUFFER);



	//You don't need to check if attachments are succesful, if they fail then application's not gonna reach here!
	LOG_STATUS("G-Buffer Draw Pass Creation is done!");
}
void Main_DrawPass::ResourceUpdatePhase() {
	if (!Is_SetupPhase_Called) {
		LOG_CRASHING("You should call RenderGraph_SetupPhase first!");
		return;
	}
	for (unsigned int i = 0; i < RG_DrawCallBuffer.size(); i++) {
		if ((RG_DrawCallBuffer[i].JoinedDrawPasses & Get_BitMaskFlag())) {
			DrawCallBuffer.push_back(i);
		}
	}

}
void Main_DrawPass::Execute() {
	if (!Is_SetupPhase_Called) {
		LOG_CRASHING("You should call RenderGraph_SetupPhase first!");
	}
	LOG_STATUS("G-Buffer Render Loop is started to run!");
	GFX->Check_Errors();

	GFXRENDERER->Bind_Framebuffer(FB);

	//Status Report!
	{
		string Before_RenderStatus;
		Before_RenderStatus.append("Mesh number that will be rendered: ");
		Before_RenderStatus.append(std::to_string(DrawCallBuffer.size()).c_str());
		LOG_STATUS(Before_RenderStatus);
	}

	//DRAW TRIANGLES
	{
		GFXRENDERER->Set_DepthTest(GFX_API::DEPTH_MODEs::DEPTH_READ_WRITE, GFX_API::DEPTH_TESTs::DEPTH_TEST_LESS);
		GFXRENDERER->Set_CullingMode(GFX_API::CULL_MODE::CULL_OFF);

		const GFX_API::DrawCall* DrawCall = nullptr;
		vector<const GFX_API::GFX_Mesh*> MESHes(DrawCallBuffer.size());
		for (unsigned int drawcall_index = 0; drawcall_index < DrawCallBuffer.size(); drawcall_index++) {
			DrawCall = &RG_DrawCallBuffer[DrawCallBuffer[drawcall_index]];
			MESHes[drawcall_index] = GFXContentManager->Find_MeshBuffer_byBufferID(DrawCall->MeshBuffer_ID);
		}

		for (unsigned int drawcall_index = 0; drawcall_index < DrawCallBuffer.size(); drawcall_index++) {
			DrawCall = &RG_DrawCallBuffer[DrawCallBuffer[drawcall_index]];
			GFX_API::Material_Instance* MATINST = (GFX_API::Material_Instance*)TuranEditor::EDITOR_FILESYSTEM->Find_ResourceIdentifier_byID(DrawCall->ShaderInstance_ID)->DATA;
			GFXRENDERER->Bind_MatInstance(MATINST);
			GFXRENDERER->DrawTriangle(MESHes[drawcall_index]);
		}
	}
	DrawCallBuffer.clear();
	
	//DRAW LINES
	{
		GFXRENDERER->Set_DepthTest(GFX_API::DEPTH_MODEs::DEPTH_READ_WRITE, GFX_API::DEPTH_TESTs::DEPTH_TEST_LEQUAL);
		GFXRENDERER->Set_CullingMode(GFX_API::CULL_MODE::CULL_OFF);
		GFXRENDERER->Set_LineWidth(5.0f);
		const GFX_API::PointLineDrawCall* PointLineDraw = nullptr;
		for (unsigned int call_index = 0; call_index < RG_PointDrawCallBuffer.size(); call_index++) {
			PointLineDraw = &RG_PointDrawCallBuffer[call_index];
			GFX_API::Material_Instance* MatInst = (GFX_API::Material_Instance*)TuranEditor::EDITOR_FILESYSTEM->Find_ResourceIdentifier_byID(PointLineDraw->ShaderInstance_ID)->DATA;
			GFXRENDERER->Bind_MatInstance(MatInst);
			if (PointLineDraw->Draw_asPoint) {
				GFXRENDERER->DrawPoint(GFXContentManager->Find_PointBuffer_byBufferID(PointLineDraw->PointBuffer_ID));
			}
			else {
				GFXRENDERER->DrawLine(GFXContentManager->Find_PointBuffer_byBufferID(PointLineDraw->PointBuffer_ID));
			}
		}
	}
	

	LOG_STATUS("G-Buffer Render Loop is finished!");
}
