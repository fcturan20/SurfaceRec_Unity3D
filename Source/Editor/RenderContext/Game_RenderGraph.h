#pragma once
#include "GFX/GFX_Includes.h"
#include "GFX/Renderer/GFX_RenderGraph.h"

namespace TuranEditor {
	class Game_RenderGraph : public GFX_API::RenderGraph
	{
	public:
		Game_RenderGraph();
		virtual void Run_RenderGraph() override;
	};
}