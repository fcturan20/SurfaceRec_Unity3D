#pragma once
#include "Editor/Editor_Includes.h"
#include "GFX/IMGUI/IMGUI_WINDOW.h"
#include "Editor/FileSystem/ResourceTypes/Resource_Identifier.h"
#include "GFX/GFX_FileSystem/Resource_Type/Material_Type_Resource.h"
#include "GFX\GFX_Core.h"
#include "Game_RenderGraph.h"

namespace TuranEditor {
#define MAX_WORLDOBJECTs 1000
#define MAX_DIRECTIONALLIGHTs 1
#define MAX_SPOTLIGHTs 10
#define MAX_POINTLIGHTs 10


	struct SURFACEMAT_PROPERTIES {
		unsigned int DIFFUSETEXTURE_ID = 0;
		unsigned int NORMALSTEXTURE_ID = 0;
		unsigned int SPECULARTEXTURE_ID = 0;
		unsigned int OPACITYTEXTURE_ID = 0;
	};

	struct Directional_Light {
		vec4 DIRECTION, COLOR;	//These variables are used as vec3 but padding forces to use as vec4
	};


	enum LIGHT_TYPE : unsigned char {
		DIRECTIONAL, POINT, SPOT
	};
	class LIGHT {
		LIGHT_TYPE TYPE;
		void* DATA;
		unsigned int LIGHT_INDEX;
	}; 

	struct POINTRENDERER {
	public:
		POINTRENDERER(unsigned int PointCount);
		vec3& GetPointPosition_byIndex(unsigned int i);
		vec4& GetPointCOLORRGBA_byIndex(unsigned int i);
		bool isPhongShadingActive = false;
		void RenderThisFrame();
		~POINTRENDERER();
	private:
		std::vector<vec3> PositionXYZs;
		std::vector<vec4> PointCOLORRGBAs;
		bool shouldRenderThisFrame = false, isResourcesUpdated = true;
		unsigned int GPUHandle = UINT32_MAX;
		friend class RenderDataManager;
	};

	class RenderDataManager {
		//Point Renderers are just storage buffers that are sent to GPU every frame and rendered with PointRendererShader material instance
		static std::vector<POINTRENDERER*> PointRenderers;
	public:
		static GFX_API::VertexAttributeLayout PositionNormal_VertexAttrib, PositionOnly_VertexAttrib;

		//Geometry Processing

		static POINTRENDERER* Create_PointRenderer(unsigned int PointCount);
		//You don't need to anything (except setting it to nullptr) with the pointer after calling this, all memory is cleared!
		static void DestroyPointRenderer(POINTRENDERER* pointrenderer);
		static GFX_API::SpecialDrawCall Create_PlaneSpecialDrawCall(vec3 Center, vec3 Tangent, vec3 Bitangent, vec2 Size, vec3 COLOR);
		static GFX_API::SpecialDrawCall Create_BoundingBoxSpecialDrawCall(vec3 BoundingMin, vec3 BoundingMax);

		//Core Renderer Datas

		static mat4* CAMERABUFFER_DATA, * WORLDOBJECTs_BUFFERDATA;
		static void* LIGHTsBUFFER_DATA;
		static unsigned int WORLDOBJECTs_GLOBALBUFFERID, MATINSTs_GLOBALBUFFERID, CAMERA_GLOBALBUFFERID, LIGHTs_GLOBALBUFFERID,
			SurfaceMatType_ID;

		static Directional_Light* DIRECTIONALLIGHTs;
		static unsigned int DIRECTIONALLIGHTs_COUNT;
		static vec3 CameraPos, FrontVec, FirstObjectPosition, FirstObjectRotation;
		static void Start_RenderingDataManagement(Game_RenderGraph* DefaultRG);
		//Pass nullptr if this is a new object in the world, pass a valid pointer if this is a mesh of an object that's already created
		//OBJECT_WORLDID should be a permanent data, because Material Instance's OBJECT_INDEX uniform data's pointer points to OBJECT_WORLDID
		static unsigned int Create_SurfaceMaterialInstance(SURFACEMAT_PROPERTIES MaterialInstance_Properties, unsigned int* OBJECT_WORLDID, unsigned int usingPhongShading = 0);
		//Returns sphere vertex buffer
		static vector<vec3> Create_Sphere(glm::vec3 Center, float radius);
		static unsigned int Create_Plane(glm::vec3 Center, glm::vec3 Tangent, glm::vec3 Bitangent, glm::vec2 Size);
		//Do the draw calls of PointRenderers, update GPU buffers etc
		static void Update_GPUResources();
	};
}