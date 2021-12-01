#pragma once
#include "Editor/Editors/PointCloudViewer.h"

class PCViewerOptions {
public:
	static void CameraOptions(PCViewer* Viewer);
	static void PCOptions(PCViewer* Viewer);
	static void ImportedFileOptions(PCViewer* Viewer);
	static void VisibilityOptions(PCViewer* Viewer);
	static void RenderGraphOptions(PCViewer* Viewer);
	static void LightingOptions(PCViewer* Viewer);
};