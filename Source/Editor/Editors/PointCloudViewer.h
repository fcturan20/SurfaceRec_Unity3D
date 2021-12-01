#pragma once
#include "Editor/Editor_Includes.h"
#include "GFX/IMGUI/IMGUI_WINDOW.h"
#include "Editor/RenderContext/Game_RenderGraph.h"
#include "Editor/RenderContext/Editor_DataManager.h"
#include "GFX/GFX_Core.h"
#include "Editor/TUBITAK/DataTypes.h"
#include "Main_Window.h"

class PCViewer : public GFX_API::IMGUI_WINDOW {
public:
	PCViewer(Game_RenderGraph* GameRenderGraph = nullptr);
	virtual void Run_Window();
	~PCViewer();


	Game_RenderGraph* RG = nullptr;
	//General Visualization Variables

	bool isCameraMoving = false;
	vec2 LastMousePos = vec2(0), Yaw_Pitch = vec2(0);
	Camera PCCamera;

	class DisplayableData {
	public:
		enum DataType : unsigned char {
			POINTCLOUD = 0,
			TRIANGLEMODEL = 1,
			ALL = 2
		};
		DataType TYPE;
		dvec3 CenterOfData = dvec3(0), BOUNDINGMIN = dvec3(0), BOUNDINGMAX = dvec3(0); double BoundingSphereRadius = 0.0;
		string PATH = "NaN", NAME = "NaN";
		bool isVisible = false;
		virtual void Display(Game_RenderGraph* RenderGraph) = 0;
	};

	class TriangleModel : public DisplayableData {
	public:
		struct TriangleModelNormals {
			vector<vector<vec3>> NonIndexed_VertexNormals;
			string NORMALNAME;
		};
		vector<TriangleModelNormals> VertexNormalTYPEs;
		vector<vector<vec3>> NonIndexed_VertexBuffers;
		vector<bool> DisplayedVertexBuffers;
		vector<unsigned int> GPUMESH_IDs;
		unsigned int RENDERINGMODE = 0;
		string INFO = "NaN";
		TriangleModel();
		virtual void Display(Game_RenderGraph* RenderGraph) override;
	};

	class PointCloud_DD : public DisplayableData {
	public:
		PointCloud PC;
		TuranEditor::POINTRENDERER* PCRenderer = nullptr;
		PointCloud_DD();
		virtual void Display(Game_RenderGraph* RenderGraph) override;
	};

	vector<DisplayableData*> DisplayableDatas;
	bool SelectListOneLine_FromDisplayableDatas(DisplayableData::DataType TYPE, unsigned int& selectedlistitemindex, unsigned int& itemsddindex, const char* ListName);
};
