#include "PointCloudViewer.h"

#include "GFX/GFX_Core.h"
#include "Editor/RenderContext/Game_RenderGraph.h"
#include "GFX/GFX_Core.h"
#include "Editor/RenderContext/Editor_DataManager.h"
#include "Editor/Editor.h"
#include <string>
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/filters/filter.h>

static std::string PointCloudPATH = "C:/Users/furka/Desktop/KinectScan.pcd", PCViewerError = "";
static unsigned int PointCloudHandle = UINT32_MAX;
static Camera PCCamera;
static PointCloud Shown_PC;
#define DISPLAYWIDTH 960
#define DISPLAYHEIGHT 540

static Game_RenderGraph* RG = nullptr;

PCViewer::PCViewer(Game_RenderGraph* RenderGraph) : IMGUI_WINDOW("PCViewer"){
	IMGUI_REGISTERWINDOW(this);
	RG = RenderGraph;
}


static bool isCameraMoving = false;
static vec2 LastMousePos = vec2(0), Yaw_Pitch = vec2(0);
static vec3 PointCloudCenter = vec3(0), PointCloudRotation(0);
static float CameraSpeed_RelativeToBoundingSphere = 1.0f;
static dvec3 CenterofPC(0); static double BoundingSphereRadiusofPC = 0.0;

void DestroyPreviousPCViewerResources() {
	if (PointCloudHandle != UINT32_MAX) {
		GFXContentManager->Unload_PointBuffer(PointCloudHandle);
		delete[] Shown_PC.PointPositions;
		delete[] Shown_PC.PointNormals;
	}
	PointCloudHandle = 0;
	PCCamera.Position = vec3(0);
	PCCamera.Front_Vector = vec3(0, 0, 1);
	CameraSpeed_RelativeToBoundingSphere = 1.0f;
	PCViewerError = "";

	Shown_PC.DifferentRepresentations.clear();
	Shown_PC.PointCount = 0;
	Shown_PC.PointNormals = nullptr;
	Shown_PC.PointPositions = nullptr;
}

void PCViewer::Run_Window() {
	if (!Is_Window_Open) {
		IMGUI_DELETEWINDOW(this);
		return;
	}
	if (!IMGUI->Create_Window("Main Window", Is_Window_Open, true)) {
		IMGUI->End_Window();
		return;
	}
	IMGUI->Input_Text("Point Cloud PATH", &PointCloudPATH);
	if (IMGUI->Button("Load PCD")) {
		pcl::PointCloud<pcl::PointXYZ> PCLCloud;
		if (pcl::io::loadPCDFile<pcl::PointXYZ>(PointCloudPATH, PCLCloud) != -1) {
			DestroyPreviousPCViewerResources();
			PCLCloud.is_dense = false;
			pcl::PointCloud<pcl::PointXYZ> noNaNPCLCloud;
			std::vector<pcl::index_t> ind;
			pcl::removeNaNFromPointCloud(PCLCloud, noNaNPCLCloud, ind);


			Shown_PC.PointCount = noNaNPCLCloud.size();
			Shown_PC.PointNormals = new vec3[Shown_PC.PointCount]{vec3(0)};
			Shown_PC.PointPositions = new vec3[Shown_PC.PointCount]{vec3(0)};
			for (unsigned int i = 0; i < Shown_PC.PointCount; i++) {
				Shown_PC.PointPositions[i] = vec3(noNaNPCLCloud.points[i].x, noNaNPCLCloud.points[i].y, noNaNPCLCloud.points[i].z);
			}
			

			//Upload the data, calculate the center and the bounding sphere radius
			vector<vec3> DATA(Shown_PC.PointCount * 2);
			for (unsigned int i = 0; i < Shown_PC.PointCount; i++) {
				DATA[i] = Shown_PC.PointPositions[i];
				CenterofPC += Shown_PC.PointPositions[i];
				DATA[i + Shown_PC.PointCount] = Shown_PC.PointNormals[i];
			}
			CenterofPC = CenterofPC / dvec3(Shown_PC.PointCount);
			for (unsigned int i = 0; i < Shown_PC.PointCount; i++) {
				double dist = length(CenterofPC - dvec3(Shown_PC.PointPositions[i]));
				if (dist > BoundingSphereRadiusofPC) { BoundingSphereRadiusofPC = dist; }
			}

			PCCamera.Position = CenterofPC - (dvec3(0, 0, 1) * dvec3(BoundingSphereRadiusofPC));
			PCCamera.Front_Vector = vec3(0, 0, 1);
			PCCamera.cameraSpeed_Base = CameraSpeed_RelativeToBoundingSphere * BoundingSphereRadiusofPC / 1000.0f;

			PointCloudHandle = GFXContentManager->Upload_PointBuffer(TuranEditor::RenderDataManager::PositionNormal_VertexAttrib, DATA.data(), Shown_PC.PointCount);
			if (PointCloudHandle == UINT32_MAX) {
				LOG_CRASHING("Uploading the point buffer to GPU has failed!");
			}
		}
		else {
			PCViewerError = "PCD file isn't found!";
		}
	}
	IMGUI->Text(PCViewerError.c_str());
	if (IMGUI->Begin_TabBar()) {
		if (IMGUI->Begin_TabItem("Point Cloud Settings")) {

			IMGUI->End_TabItem();
		}

		if (IMGUI->Begin_TabItem("Camera Settings")) {
			if (IMGUI->Slider_Float("Camera Speed", &CameraSpeed_RelativeToBoundingSphere, 0.0f, 1.0f)) {
				PCCamera.cameraSpeed_Base = CameraSpeed_RelativeToBoundingSphere * BoundingSphereRadiusofPC / 1000.0f;
			}
		}

		IMGUI->End_TabBar();
	}
	if (PointCloudHandle != UINT32_MAX) {
		GFX_API::PointLineDrawCall LDC;
		LDC.Draw_asPoint = true;
		LDC.PointBuffer_ID = PointCloudHandle;
		LDC.ShaderInstance_ID = TuranEditor::RenderDataManager::ShadedPoint_MatInst;
		RG->Register_PointDrawCall(LDC);
	}

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
	TuranEditor::RenderDataManager::FirstObjectPosition = PointCloudCenter;
	TuranEditor::RenderDataManager::FirstObjectRotation = PointCloudRotation;
	TuranEditor::RenderDataManager::CameraPos = PCCamera.Position;
	TuranEditor::RenderDataManager::FrontVec = PCCamera.Front_Vector;

	GFX->Register_RenderGraph(RG);


	IMGUI->End_Window();
}

PCViewer::~PCViewer() {
	IMGUI_DELETEWINDOW(this);
}