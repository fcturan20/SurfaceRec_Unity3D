#pragma once
#include "Editor/Editor_Includes.h"
#include "GFX/IMGUI/IMGUI_WINDOW.h"
#include "Editor/RenderContext/Game_RenderGraph.h"
#include "GFX/GFX_Core.h"
#include "Editor/Ýþ/DataTypes.h"
#include "Main_Window.h"


class PCViewer : public GFX_API::IMGUI_WINDOW {
public:
	PCViewer(Game_RenderGraph* GameRenderGraph = nullptr);
	virtual void Run_Window();
	~PCViewer();
};
