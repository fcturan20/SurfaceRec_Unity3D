#pragma once
#include "Editor/Editor_Includes.h"
#include "GFX/IMGUI/IMGUI_WINDOW.h"
#include "Editor/FileSystem/ResourceTypes/Resource_Identifier.h"
#include "GFX/GFX_FileSystem/Resource_Type/Material_Type_Resource.h"

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

	struct Spot_Light {
		vec4 POSITION;	//These variables are used as vec3 but padding forces to use as vec4
	};

	struct Point_Light {
		vec4 POSITION;	//These variables are used as vec3 but padding forces to use as vec4
	};

	enum LIGHT_TYPE : unsigned char {
		DIRECTIONAL, POINT, SPOT
	};
	class LIGHT {
		LIGHT_TYPE TYPE;
		void* DATA;
		unsigned int LIGHT_INDEX;
	};

	class Editor_RendererDataManager {

		static Bitset WORLDOBJECT_IDs;
		static unsigned int Create_OBJECTINDEX();
		static void Delete_OBJECTINDEX(unsigned int INDEX);
		static void Load_SurfaceMaterialType();
		static void Load_PointLineMaterialType();

	public:
		static mat4* CAMERABUFFER_DATA, * WORLDOBJECTs_BUFFERDATA;
		static void* LIGHTsBUFFER_DATA, *GEODESICSBUFFERDATA;
		static unsigned int WORLDOBJECTs_GLOBALBUFFERID, MATINSTs_GLOBALBUFFERID, CAMERA_GLOBALBUFFERID, LIGHTs_GLOBALBUFFERID,
			SurfaceMatType_ID, GeodesicDistancesBuffer_ID;

		static Directional_Light* DIRECTIONALLIGHTs;
		static Spot_Light* SPOTLIGHTs;
		static Point_Light* POINTLIGHTs;
		static unsigned int DIRECTIONALLIGHTs_COUNT, SPOTLIGHTs_COUNT, POINTLIGHTs_COUNT, PointLineMaterialID;
		static vec3 CameraPos, FrontVec, FirstObjectPosition, FirstObjectRotation;
		static void Start_RenderingDataManagement();
		//Pass nullptr if this is a new object in the world, pass a valid pointer if this is a mesh of an object that's already created
		//OBJECT_WORLDID should be a permanent data, because Material Instance's OBJECT_INDEX uniform data's pointer points to OBJECT_WORLDID
		static unsigned int Create_SurfaceMaterialInstance(SURFACEMAT_PROPERTIES MaterialInstance_Properties, unsigned int* OBJECT_WORLDID);
		static void UpdateGeodesicDistances(const void* DATA, unsigned int DATASIZE);
		static void Update_GPUResources();
	};
}