#pragma once
#include "Editor_Includes.h"
#include "Editors\Main_Window.h"
#include "Editors\PointCloudViewer.h"
#include "RenderContext/Game_RenderGraph.h"
#include "GFX/IMGUI/IMGUI_Core.h"
#include "GFX/GFX_Core.h"
#include "FileSystem/EditorFileSystem_Core.h"
#include "Staj/DijkstraSPFAlgorithm.h"

#include "FileSystem/ResourceImporters/Model_Loader.h"
#include "Editor/RenderContext/Editor_DataManager.h"
#include "TuranAPI/Profiler_Core.h"
#include "Editor/FileSystem/ResourceTypes/ResourceTYPEs.h"

//Is
#include "Editor/Ýþ/DataLoader.h"
#include "Editor/Ýþ/DataTypes.h"
#include "Editor/Ýþ/Algorithms.h"
#include "Editor/Ýþ/Algorithms_UnitTests.h"

namespace TuranEditor {
	class Editor_System {
		TuranAPI::Logging::Logger LOGGING;
		Editor_FileSystem FileSystem;
		TuranAPI::Profiler_System Profiler;
		static bool Should_Close;
		static float DeltaTime_inmilliseconds;
	public:
		Editor_System();
		~Editor_System();
		static void Take_Inputs();
		static bool Should_EditorClose();
		//This is in milliseconds
		static float Get_DeltaTime();
		static void Set_DeltaTime(float delta);
	};
}