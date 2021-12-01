#pragma once
#include "GFX/GFX_Includes.h"
#include "GFX/Renderer/GFX_RenderGraph.h"

class Game_RenderGraph : public GFX_API::RenderGraph
{
public:
	bool shouldPC_DepthWrite = false;
	Game_RenderGraph();
	virtual void Run_RenderGraph() override;
	void* GetDepthBuffer(unsigned int& DATASIZE);
};