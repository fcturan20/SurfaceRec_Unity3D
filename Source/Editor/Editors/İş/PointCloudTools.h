#pragma once
#include "Editor/Editors/PointCloudViewer.h"

class PointCloudTools {
public:
	static void ObjectDetection(PCViewer* Viewer);
	static void CoreAlgorithms(PCViewer* Viewer);
	static void NormalEstimation(PCViewer* Viewer);
	static void SurfaceReconstruction(PCViewer* Viewer);
};