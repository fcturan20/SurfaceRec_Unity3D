#pragma once
#include "GFX/GFX_Includes.h"
#include "GFX/Renderer/GFX_Resource.h"
#include "GFX/Renderer/GFX_RenderGraph.h"
#include "GFX/GFX_FileSystem/Resource_Type/Material_Type_Resource.h"


class Main_DrawPass : public GFX_API::DrawPass {
	const GFX_API::Framebuffer* FB;
	void Create_LineRendererMatInst();
public:
	bool shouldPC_DepthWrite = true;
	Main_DrawPass(const vector<GFX_API::DrawCall>& RG_DrawCalls, const vector<GFX_API::PointLineDrawCall>& i_RG_PointDrawCallBuffer, const vector<GFX_API::SpecialDrawCall>& i_RGSpecialDCBuffer, vector<GFX_API::Framebuffer::RT_SLOT>& Needed_RTSlots);

	static unsigned int Get_BitMaskFlag();

	virtual void RenderGraph_SetupPhase(vector<GFX_API::Framebuffer::RT_SLOT>& RTs) override;
	virtual void ResourceUpdatePhase() override;
	virtual void Execute() override;
};
