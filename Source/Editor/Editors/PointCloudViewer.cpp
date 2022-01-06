#include "PointCloudViewer.h"
#include "Biligoba/PointCloudImports.h"
#include "Biligoba/PointCloudTools.h"
#include "Biligoba/Options.h"

#include "GFX/GFX_Core.h"
#include "Editor/Editor.h"
#include <string>

#define DISPLAYWIDTH 960
#define DISPLAYHEIGHT 540

//0 is triangle, 1 is lighting, 2 is normal
unsigned int SurfaceMatInst_RenderModes[3] = { UINT32_MAX };


PCViewer::PCViewer(Game_RenderGraph* RenderGraph) : IMGUI_WINDOW("PCViewer"){
	IMGUI_REGISTERWINDOW(this);
	RG = RenderGraph;
	for (unsigned int rendermode = 0; rendermode < 3; rendermode++) {
		SurfaceMatInst_RenderModes[rendermode] = TuranEditor::RenderDataManager::Create_SurfaceMaterialInstance(TuranEditor::SURFACEMAT_PROPERTIES(), nullptr, rendermode);
	}
}


void PCViewer::Run_Window() {
	static bool Display_PC = true;
	static unsigned char MenuIndex = 255, MenuItemIndex = 255;
	if (!Is_Window_Open) {
		IMGUI_DELETEWINDOW(this);
		return;
	}
	if (!IMGUI->Create_Window("Main Window", Is_Window_Open, true)) {
		IMGUI->End_Window();
		return;
	}
	if (IMGUI->Begin_Menubar()) {
		if (IMGUI->Begin_Menu("Import/Export")) {
			MenuIndex = 0;
			if (IMGUI->Menu_Item("Import Model")) { MenuItemIndex = 0; }
			if (IMGUI->Menu_Item("Import .PCD")) { MenuItemIndex = 1; }
			if (IMGUI->Menu_Item("Import/Export .PC")) { MenuItemIndex = 2; }
			IMGUI->End_Menu();
		}

		if (IMGUI->Begin_Menu("Tools")) {
			MenuIndex = 1;
			if (IMGUI->Menu_Item("Object Detection")) { MenuItemIndex = 0; }
			if (IMGUI->Menu_Item("Core Algorithms")) { MenuItemIndex = 1; }
			if (IMGUI->Menu_Item("Normal Estimation")) { MenuItemIndex = 2; }
			if (IMGUI->Menu_Item("Surface Reconstruction")) { MenuItemIndex = 3; }
			IMGUI->End_Menu();
		}

		if (IMGUI->Begin_Menu("Options")) {
			MenuIndex = 2;
			if (IMGUI->Menu_Item("Camera")) { MenuItemIndex = 0; }
			if (IMGUI->Menu_Item("Point Cloud")) { MenuItemIndex = 1; }
			if (IMGUI->Menu_Item("Imported File")) { MenuItemIndex = 2; }
			if (IMGUI->Menu_Item("Visibility")) { MenuItemIndex = 3; }
			if (IMGUI->Menu_Item("RenderGraph")) { MenuItemIndex = 4; }
			if (IMGUI->Menu_Item("Lighting")) { MenuItemIndex = 5; }

			IMGUI->End_Menu();
		}

		IMGUI->End_Menubar();
	}
	switch (MenuIndex) {
	case 0:
		if (MenuItemIndex == 0) { PointCloudImports::ImportPolygonalModel(this);}
		if (MenuItemIndex == 1) { PointCloudImports::ImportPCDFile(this); }
		if (MenuItemIndex == 2) { PointCloudImports::ImportExport_DGSFile(this); }
		break;
	case 1:
		if (MenuItemIndex == 0) { PointCloudTools::ObjectDetection(this); }
		if (MenuItemIndex == 1) { PointCloudTools::CoreAlgorithms(this); }
		if (MenuItemIndex == 2) { PointCloudTools::NormalEstimation(this); }
		if (MenuItemIndex == 3) { PointCloudTools::SurfaceReconstruction(this); }
		break;
	case 2:
		if (MenuItemIndex == 0) { PCViewerOptions::CameraOptions(this); }
		if (MenuItemIndex == 1) { PCViewerOptions::PCOptions(this); }
		if (MenuItemIndex == 2) { PCViewerOptions::ImportedFileOptions(this); }
		if (MenuItemIndex == 3) { PCViewerOptions::VisibilityOptions(this); }
		if (MenuItemIndex == 4) { PCViewerOptions::RenderGraphOptions(this); }
		if (MenuItemIndex == 5) { PCViewerOptions::LightingOptions(this); }
		break;
	}
	IMGUI->Text(("Camera Front Vector X: " + std::to_string(PCCamera.Front_Vector.x) + " Y: " + std::to_string(PCCamera.Front_Vector.y) + " Z: " +
		std::to_string(PCCamera.Front_Vector.z)).c_str());
	IMGUI->Text(("Camera Position X: " + std::to_string(PCCamera.Position.x) + " Y: " + std::to_string(PCCamera.Position.y) + " Z: " +
		std::to_string(PCCamera.Position.z)).c_str());


	vec2 MousePos = IMGUI->GetMouseWindowPos() - IMGUI->GetItemWindowPos();
	unsigned int DisplayTexture = GFXContentManager->Find_Framebuffer_byGFXID(((GFX_API::DrawPass*)RG->Get_RenderNodes()[0])->Get_FramebufferID())->BOUND_RTs[0].RT_ID;
	IMGUI->Display_Texture(DisplayTexture, DISPLAYWIDTH, DISPLAYHEIGHT, true);


	if (MousePos.x >= 0.0f && MousePos.y >= 0.0f && DISPLAYWIDTH - MousePos.x > 0.0f && DISPLAYHEIGHT - MousePos.y > 0.0f &&
		GFX->IsMouse_Clicked(GFX_API::MOUSE_BUTTONs::MOUSE_RIGHT_CLICK)) {
		if (!isCameraMoving) {
			LastMousePos = MousePos;
			isCameraMoving = true;
		}

		vec2 MouseOffset = MousePos - LastMousePos;
		MouseOffset.y = -MouseOffset.y;
		//Sensitivity
		MouseOffset *= 0.1f;
		Yaw_Pitch += MouseOffset;
		if (Yaw_Pitch.y > 89.0f) {
			Yaw_Pitch.y = 89.0f;
		}
		else if (Yaw_Pitch.y < -89.0f) {
			Yaw_Pitch.y = -89.0f;
		}

		LastMousePos = MousePos;
		PCCamera.Update(Yaw_Pitch.x, Yaw_Pitch.y);
	}
	else {
		isCameraMoving = false;
	}
	for (unsigned int i = 0; i < DisplayableDatas.size(); i++) {
		if (DisplayableDatas[i]->isVisible) { DisplayableDatas[i]->Display(RG); }
	}
	TuranEditor::RenderDataManager::FirstObjectPosition = vec3(0);
	TuranEditor::RenderDataManager::FirstObjectRotation = vec3(0);
	TuranEditor::RenderDataManager::CameraPos = PCCamera.Position;
	TuranEditor::RenderDataManager::FrontVec = PCCamera.Front_Vector;

	GFX->Register_RenderGraph(RG);


	IMGUI->End_Window();
}

//If there is no displayable data of specified kind, itemsddindex will be UINT32_MAX
bool PCViewer::SelectListOneLine_FromDisplayableDatas(DisplayableData::DataType TYPE, unsigned int& selectedlistitemindex, unsigned int& itemsddindex, const char* ListName) {
	vector<unsigned int> PCs;
	for (unsigned int i = 0; i < DisplayableDatas.size(); i++) {
		for (unsigned int namecheck_i = i + 1; namecheck_i < DisplayableDatas.size(); namecheck_i++) {
			if (DisplayableDatas[i]->NAME == DisplayableDatas[namecheck_i]->NAME) { DisplayableDatas[namecheck_i]->NAME += "_Copy(1)"; }
		}
	}
	if (TYPE == DisplayableData::DataType::ALL) {
		for (unsigned int i = 0; i < DisplayableDatas.size(); i++) {
			PCs.push_back(i);
		}
	}
	else {
		for (unsigned int i = 0; i < DisplayableDatas.size(); i++) {
			if (DisplayableDatas[i]->TYPE == TYPE) {
				PCs.push_back(i);
			}
		}
	}
	vector<const char*> PC_NAMEs(PCs.size(), "NaN");
	for (unsigned int i = 0; i < PCs.size(); i++) {
		PC_NAMEs[i] = DisplayableDatas[PCs[i]]->NAME.c_str();
	}
	if (!PC_NAMEs.size()) {
		itemsddindex = UINT32_MAX;
		return false;
	}
	selectedlistitemindex = std::min(selectedlistitemindex, unsigned int(PC_NAMEs.size() - 1));
	bool r = IMGUI->SelectList_OneLine(ListName, &selectedlistitemindex, PC_NAMEs);
	itemsddindex = PCs[selectedlistitemindex];
	return r;
}


PCViewer::TriangleModel::TriangleModel() { TYPE = TRIANGLEMODEL; }
void PCViewer::TriangleModel::Display(Game_RenderGraph* RenderGraph) {
	/*
	if (NonIndexed_VertexBuffers.size() != GPUMESH_IDs.size()) {
		LOG_CRASHING("Triangle model " + NAME + " has unmatching GPUMESH_IDs-NonIndexed_VertexBuffers arrays!");
	}
	if (DisplayedVertexBuffers.size() != NonIndexed_VertexBuffers.size()) {
		LOG_CRASHING("Triangle model " + NAME + " has unmatching DisplayedVertexBuffers-NonIndexed_VertexBuffers arrays!");
	}*/
	for (unsigned int i = 0; i < DisplayedVertexBuffers.size(); i++) {
		if (!DisplayedVertexBuffers[i]) {
			continue;
		}
		 
		GFX_API::DrawCall DC;
		DC.JoinedDrawPasses = 0xFF;
		DC.MeshBuffer_ID = GPUMESH_IDs[i];
		DC.ShaderInstance_ID = SurfaceMatInst_RenderModes[RENDERINGMODE];
		RenderGraph->Register_DrawCall(DC);
	}
}

PCViewer::PointCloud_DD::PointCloud_DD() { TYPE = POINTCLOUD; }
void PCViewer::PointCloud_DD::Display(Game_RenderGraph* RenderGraph) {
	PCRenderer->RenderThisFrame();
}

PCViewer::~PCViewer() {
	IMGUI_DELETEWINDOW(this);
}