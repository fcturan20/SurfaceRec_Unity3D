#include "Options.h"



void PCViewerOptions::CameraOptions(PCViewer* Viewer) {
	static unsigned int listitemindex = 0, Relative_DisplayableDataIndex = 0;
	static float CameraSpeed_RelativeToBoundingSphere_old = 1.0f, CameraSpeed_RelativeToBoundingSphere_new = 1.0f;
	if (Viewer->SelectListOneLine_FromDisplayableDatas(PCViewer::DisplayableData::ALL, listitemindex, Relative_DisplayableDataIndex, "Camera Speed Relative To")) {
		Viewer->PCCamera.cameraSpeed_Base = CameraSpeed_RelativeToBoundingSphere_old * Viewer->DisplayableDatas[Relative_DisplayableDataIndex]->BoundingSphereRadius / 1000.0f;
	}
	
	if (IMGUI->Slider_Float("Camera Speed relative to the Bounding Sphere", &CameraSpeed_RelativeToBoundingSphere_new, 0.0f, 10.0f)) {
		Viewer->PCCamera.cameraSpeed_Base *= CameraSpeed_RelativeToBoundingSphere_new / CameraSpeed_RelativeToBoundingSphere_old;
		CameraSpeed_RelativeToBoundingSphere_old = CameraSpeed_RelativeToBoundingSphere_new;
	}
	if (IMGUI->Button("Relocate to bounding sphere")) {
		Viewer->PCCamera.Position = Viewer->DisplayableDatas[Relative_DisplayableDataIndex]->CenterOfData - (dvec3(0, 0, 1) * dvec3(Viewer->DisplayableDatas[Relative_DisplayableDataIndex]->BoundingSphereRadius));
		Viewer->PCCamera.Front_Vector = vec3(0, 0, 1);
	}
}



void PCViewerOptions::PCOptions(PCViewer* Viewer) {
	static unsigned int selectedlistitemindex = 0, selectedddindex = 0;
	Viewer->SelectListOneLine_FromDisplayableDatas(PCViewer::DisplayableData::POINTCLOUD, selectedlistitemindex, selectedddindex, "Point Cloud");
	if (selectedddindex == UINT32_MAX) { return; }
	PCViewer::PointCloud_DD* pc_dd = static_cast<PCViewer::PointCloud_DD*>(Viewer->DisplayableDatas[selectedddindex]);
	if (pc_dd->PC.PointNormals.size()) {
		vector<const char*> NORMALLIST_names(pc_dd->PC.PointNormals.size());
		for (unsigned int i = 0; i < NORMALLIST_names.size(); i++) {
			NORMALLIST_names[i] = pc_dd->PC.PointNormals[i].NAME.c_str();
		}
		static unsigned int selectedNormalListIndex = 0;
		selectedNormalListIndex = std::min(selectedNormalListIndex, unsigned int(NORMALLIST_names.size() - 1));
		if (IMGUI->SelectList_OneLine("Normal List to Visualize", &selectedNormalListIndex, NORMALLIST_names)) {
			for (unsigned int i = 0; i < pc_dd->PC.PointCount; i++) {
				pc_dd->PCRenderer->GetPointCOLORRGBA_byIndex(i) = vec4(((pc_dd->PC.PointNormals[selectedNormalListIndex].Normals[i]) + vec3(1.0)) / vec3(2.0), 1.0f);
			}
		}
	}
}



void PCViewerOptions::ImportedFileOptions(PCViewer* Viewer) {

}



void PCViewerOptions::VisibilityOptions(PCViewer* Viewer) {
	static unsigned int SelectedListItemIndex = 0, selectedddindex = 0;
	Viewer->SelectListOneLine_FromDisplayableDatas(PCViewer::DisplayableData::ALL, SelectedListItemIndex, selectedddindex, "Displayable Data");

	if (selectedddindex == UINT32_MAX) {
		return;
	}
	
	IMGUI->Checkbox("Display", &Viewer->DisplayableDatas[selectedddindex]->isVisible);
	if (IMGUI->Button("Destroy Object")) {
		Viewer->DisplayableDatas.erase(Viewer->DisplayableDatas.begin() + selectedddindex);
		return;
	}
	if (!Viewer->DisplayableDatas[selectedddindex]->isVisible) { return; }
	if (Viewer->DisplayableDatas[selectedddindex]->TYPE == PCViewer::DisplayableData::TRIANGLEMODEL) {
		PCViewer::TriangleModel* TRIMODEL = static_cast<PCViewer::TriangleModel*>(Viewer->DisplayableDatas[selectedddindex]);
		unsigned int rendermode = TRIMODEL->RENDERINGMODE;
		if (IMGUI->SelectList_OneLine("Rendering Mode", &rendermode, { "Triangle", "Lighting", "Normal" })) { TRIMODEL->RENDERINGMODE = rendermode; }

		Bitset checkboxlist((TRIMODEL->DisplayedVertexBuffers.size() / 8) + 1);
		for (unsigned int i = 0; i < TRIMODEL->DisplayedVertexBuffers.size(); i++) {
			if (TRIMODEL->DisplayedVertexBuffers[i]) { checkboxlist.SetBit_True(i); }
			else { checkboxlist.SetBit_False(i); }
		}
		vector<string> names(TRIMODEL->DisplayedVertexBuffers.size());
		for (unsigned int i = 0; i < TRIMODEL->DisplayedVertexBuffers.size(); i++) {
			names[i] = to_string(i);
		}
		IMGUI->CheckListBox("Mesh Visibilities", &checkboxlist, &names);
		for (unsigned int i = 0; i < TRIMODEL->DisplayedVertexBuffers.size(); i++) {
			TRIMODEL->DisplayedVertexBuffers[i] = checkboxlist.GetBit_Value(i);
		}
	}
	if (Viewer->DisplayableDatas[selectedddindex]->TYPE == PCViewer::DisplayableData::POINTCLOUD) {
		PCViewer::PointCloud_DD* PC = static_cast<PCViewer::PointCloud_DD*>(Viewer->DisplayableDatas[selectedddindex]);
		IMGUI->Checkbox("Shading Active", &PC->PCRenderer->isPhongShadingActive);
	}
}

void PCViewerOptions::RenderGraphOptions(PCViewer* Viewer) {
	IMGUI->Checkbox("Point Renderer Write Depth", &Viewer->RG->shouldPC_DepthWrite);
}


void PCViewerOptions::LightingOptions(PCViewer* Viewer) {
	static vec4 LightDirection = TuranEditor::RenderDataManager::DIRECTIONALLIGHTs[0].DIRECTION;
	static vec4 LightColor = TuranEditor::RenderDataManager::DIRECTIONALLIGHTs[0].COLOR;
	IMGUI->Slider_Vec4("Direction", &LightDirection, -1.0, 1.0);
	IMGUI->Slider_Vec4("Color", &LightColor, 0.0, 1.0);
	TuranEditor::RenderDataManager::DIRECTIONALLIGHTs[0].DIRECTION = LightDirection;
	TuranEditor::RenderDataManager::DIRECTIONALLIGHTs[0].COLOR = LightColor;
}