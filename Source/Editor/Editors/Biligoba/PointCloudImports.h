#pragma once
#include "Editor/Editor_Includes.h"
#include "Editor/TUBITAK/DataTypes.h"
#include "Editor/RenderContext/Game_RenderGraph.h"
#include "Editor/Editors/PointCloudViewer.h"


class PointCloudImports {
public:
	static void ImportPolygonalModel(PCViewer* Viewer);
	static void ImportPCDFile(PCViewer* Viewer);
	static void ImportExport_DGSFile(PCViewer* Viewer);
};
