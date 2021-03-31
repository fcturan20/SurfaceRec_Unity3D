#include "Editor_DataManager.h"
#include "Editor/FileSystem/EditorFileSystem_Core.h"
//To show import status
#include "Editor/Editors/Status_Window.h"
//To compile shader program!
#include "GFX/GFX_Core.h"
#include "Editor/FileSystem/ResourceTypes/ResourceTYPEs.h"

using namespace GFX_API;


namespace TuranEditor {
	unsigned int Editor_RendererDataManager::WORLDOBJECTs_GLOBALBUFFERID, Editor_RendererDataManager::MATINSTs_GLOBALBUFFERID, Editor_RendererDataManager::CAMERA_GLOBALBUFFERID, Editor_RendererDataManager::LIGHTs_GLOBALBUFFERID,
		Editor_RendererDataManager::SurfaceMatType_ID, Editor_RendererDataManager::GeodesicDistancesBuffer_ID;
	Directional_Light* Editor_RendererDataManager::DIRECTIONALLIGHTs = new Directional_Light[MAX_DIRECTIONALLIGHTs];
	Spot_Light* Editor_RendererDataManager::SPOTLIGHTs = new Spot_Light[MAX_SPOTLIGHTs];
	Point_Light* Editor_RendererDataManager::POINTLIGHTs = new Point_Light[MAX_POINTLIGHTs];
	unsigned int Editor_RendererDataManager::DIRECTIONALLIGHTs_COUNT = 0, Editor_RendererDataManager::SPOTLIGHTs_COUNT = 0, Editor_RendererDataManager::POINTLIGHTs_COUNT = 0, Editor_RendererDataManager::PointLineMaterialID = 0;
	void* Editor_RendererDataManager::LIGHTsBUFFER_DATA = new bool[364], * Editor_RendererDataManager::GEODESICSBUFFERDATA = new bool[30002 * 4];
	Bitset Editor_RendererDataManager::WORLDOBJECT_IDs(100);
	vec3 Editor_RendererDataManager::CameraPos(0, 0, -3), Editor_RendererDataManager::FrontVec(0, 0, 1)
		, Editor_RendererDataManager::FirstObjectPosition(0,0,0), Editor_RendererDataManager::FirstObjectRotation(-90.0f, 0.0f, -90.0f);
	mat4* Editor_RendererDataManager::CAMERABUFFER_DATA = new mat4[2],
		*Editor_RendererDataManager::WORLDOBJECTs_BUFFERDATA = new mat4[MAX_WORLDOBJECTs];
	void Editor_RendererDataManager::Load_SurfaceMaterialType() {
		//Surface Material Type has 6 uniforms: uint MATINST_INDEX, OBJECT_INDEX; sampler2D DIFFUSETEXTURE, NORMALTEXTURE, SPECULARTEXTURE, OPACITYTEXTURE.
		//MATINST_INDEX will be used to access bool TEXTUREMASK to specify the shading path
		//OBJECT_INDEX will be used to access model's local->world transformation matrix
		Material_Type* Surface_MatType = new Material_Type;

		/*
		//MATINST_INDEX uniform
		{
			Material_Uniform MatInst_Index;
			MatInst_Index.VARIABLE_NAME = "MATINST_INDEX";
			MatInst_Index.VARIABLE_TYPE = GFX_API::UNIFORMTYPE::VAR_UINT32;
			MatInst_Index.DATA = nullptr;
			Surface_MatType->UNIFORMs.push_back(MatInst_Index);
		}
		*/

		//OBJECT_INDEX uniform
		{
			Material_Uniform Object_Index;
			Object_Index.VARIABLE_NAME = "OBJECT_INDEX";
			Object_Index.VARIABLE_TYPE = GFX_API::DATA_TYPE::VAR_UINT32;
			Object_Index.DATA = nullptr;
			Surface_MatType->UNIFORMs.push_back(Object_Index);
		}
		//DIFFUSETEXTURE uniform
		{
			Texture_Access DIFFUSE_TEXTURE;
			DIFFUSE_TEXTURE.DIMENSIONs = TEXTURE_DIMENSIONs::TEXTURE_2D;
			DIFFUSE_TEXTURE.CHANNELs = TEXTURE_CHANNELs::API_TEXTURE_RGB8UB;
			DIFFUSE_TEXTURE.OP_TYPE = OPERATION_TYPE::READ_ONLY;
			DIFFUSE_TEXTURE.ACCESS_TYPE = TEXTURE_ACCESS::SAMPLER_OPERATION;
			DIFFUSE_TEXTURE.BINDING_POINT = 0;
			DIFFUSE_TEXTURE.TEXTURE_ID = 0;
			Surface_MatType->TEXTUREs.push_back(DIFFUSE_TEXTURE);
		}
		//NORMALSTEXTURE uniform
		{
			Texture_Access NORMALSTEXTURE;
			NORMALSTEXTURE.DIMENSIONs = TEXTURE_DIMENSIONs::TEXTURE_2D;
			NORMALSTEXTURE.CHANNELs = TEXTURE_CHANNELs::API_TEXTURE_RGB8UB;
			NORMALSTEXTURE.OP_TYPE = OPERATION_TYPE::READ_ONLY;
			NORMALSTEXTURE.ACCESS_TYPE = TEXTURE_ACCESS::SAMPLER_OPERATION;
			NORMALSTEXTURE.BINDING_POINT = 1;
			NORMALSTEXTURE.TEXTURE_ID = 0;
			Surface_MatType->TEXTUREs.push_back(NORMALSTEXTURE);
		}
		//SPECULARTEXTURE uniform
		{
			Texture_Access SPECULARTEXTURE;
			SPECULARTEXTURE.DIMENSIONs = TEXTURE_DIMENSIONs::TEXTURE_2D;
			SPECULARTEXTURE.CHANNELs = TEXTURE_CHANNELs::API_TEXTURE_RGB8UB;
			SPECULARTEXTURE.OP_TYPE = OPERATION_TYPE::READ_ONLY;
			SPECULARTEXTURE.ACCESS_TYPE = TEXTURE_ACCESS::SAMPLER_OPERATION;
			SPECULARTEXTURE.BINDING_POINT = 2;
			SPECULARTEXTURE.TEXTURE_ID = 0;
			Surface_MatType->TEXTUREs.push_back(SPECULARTEXTURE);
		}
		//OPACITYTEXTURE uniform
		{
			Texture_Access OPACITYTEXTURE;
			OPACITYTEXTURE.DIMENSIONs = TEXTURE_DIMENSIONs::TEXTURE_2D;
			OPACITYTEXTURE.CHANNELs = TEXTURE_CHANNELs::API_TEXTURE_RGB8UB;
			OPACITYTEXTURE.OP_TYPE = OPERATION_TYPE::READ_ONLY;
			OPACITYTEXTURE.ACCESS_TYPE = TEXTURE_ACCESS::SAMPLER_OPERATION;
			OPACITYTEXTURE.BINDING_POINT = 3;
			OPACITYTEXTURE.TEXTURE_ID = 0;
			Surface_MatType->TEXTUREs.push_back(OPACITYTEXTURE);
		}
		

		DIRECTIONALLIGHTs[0].COLOR = vec4(1, 0.5f, 0.2f, 0);
		DIRECTIONALLIGHTs[0].DIRECTION = vec4(0.5f, 0.7f, 0.9f, 0);

		//Global Buffers

		//Camera Buffer
		{
			mat4 proj_mat;
			proj_mat = perspective(radians(45.0f), float(1920.0f / 1080.0f), 0.1f, 10000.0f);
			mat4 view_mat;
			view_mat = lookAt(CameraPos, FrontVec + CameraPos, vec3(0, 1, 0));
			CAMERABUFFER_DATA[0] = proj_mat;
			CAMERABUFFER_DATA[1] = view_mat;
			CAMERA_GLOBALBUFFERID = GFXContentManager->Create_GlobalBuffer("VIEW_MATRICES", CAMERABUFFER_DATA, 128, GFX_API::BUFFER_VISIBILITY::CPUREADWRITE_GPUREADWRITE);
		}
		//World Objects Matrices Buffer
		{
			WORLDOBJECTs_GLOBALBUFFERID = GFXContentManager->Create_GlobalBuffer("MODEL_MATRICES", WORLDOBJECTs_BUFFERDATA, 16000, GFX_API::BUFFER_VISIBILITY::CPUREADWRITE_GPUREADWRITE);
		}
		//Lights Buffer
		{
			memcpy(LIGHTsBUFFER_DATA, DIRECTIONALLIGHTs, 32);
			memcpy((char*)LIGHTsBUFFER_DATA + 32, POINTLIGHTs, 160);
			memcpy((char*)LIGHTsBUFFER_DATA + 192, SPOTLIGHTs, 160);
			memcpy((char*)LIGHTsBUFFER_DATA + 352, &DIRECTIONALLIGHTs_COUNT, 4);
			memcpy((char*)LIGHTsBUFFER_DATA + 356, &POINTLIGHTs_COUNT, 4);
			memcpy((char*)LIGHTsBUFFER_DATA + 360, &SPOTLIGHTs_COUNT, 4);
			LIGHTs_GLOBALBUFFERID = GFXContentManager->Create_GlobalBuffer("LIGHTs", LIGHTsBUFFER_DATA, 364, GFX_API::BUFFER_VISIBILITY::CPUREADWRITE_GPUREADWRITE);
		}
		//Geodesic Distance Buffer
		{
			GeodesicDistancesBuffer_ID = GFXContentManager->Create_GlobalBuffer("GEOs", GEODESICSBUFFERDATA, 30002 * 4, GFX_API::BUFFER_VISIBILITY::CPUREADWRITE_GPUREADWRITE);
		}

		//GLOBAL BUFFER BINDING
		{
			GlobalBuffer_Access WORLDOBJECTs_ACCESS;
			WORLDOBJECTs_ACCESS.ACCESS_TYPE = OPERATION_TYPE::READ_ONLY;
			WORLDOBJECTs_ACCESS.BUFFER_ID = WORLDOBJECTs_GLOBALBUFFERID;
			Surface_MatType->GLOBALBUFFERs.push_back(WORLDOBJECTs_ACCESS);

			GlobalBuffer_Access CAMERA_ACCESS;
			CAMERA_ACCESS.ACCESS_TYPE = OPERATION_TYPE::READ_ONLY;
			CAMERA_ACCESS.BUFFER_ID = CAMERA_GLOBALBUFFERID;
			Surface_MatType->GLOBALBUFFERs.push_back(CAMERA_ACCESS);

			GlobalBuffer_Access LIGHTs_ACCESS;
			LIGHTs_ACCESS.ACCESS_TYPE = OPERATION_TYPE::READ_ONLY;
			LIGHTs_ACCESS.BUFFER_ID = LIGHTs_GLOBALBUFFERID;
			Surface_MatType->GLOBALBUFFERs.push_back(LIGHTs_ACCESS);

			GlobalBuffer_Access GEODESICS_ACCESS;
			LIGHTs_ACCESS.ACCESS_TYPE = OPERATION_TYPE::READ_ONLY;
			LIGHTs_ACCESS.BUFFER_ID = GeodesicDistancesBuffer_ID;
			Surface_MatType->GLOBALBUFFERs.push_back(LIGHTs_ACCESS);
		}


		//Load Vertex Shader
		unsigned int VSSOURCE_ID = 0;
		{
			LOG_STATUS("Loading Vertex Shader!\n");
			string* VERTEXSHADER_SOURCE = TAPIFILESYSTEM::Read_TextFile("C:/dev/StajRenderer/Content/SurfaceUberShader.vert");
			ShaderSource_Resource* VertexShader = new ShaderSource_Resource;
			VertexShader->LANGUAGE = GFX_API::SHADER_LANGUAGEs::GLSL;
			VertexShader->STAGE = GFX_API::SHADER_STAGE::VERTEXSTAGE;
			VertexShader->SOURCE_CODE = *VERTEXSHADER_SOURCE;

			Resource_Identifier* RESOURCE = new Resource_Identifier;
			RESOURCE->TYPE = RESOURCETYPEs::GFXAPI_SHADERSOURCE;
			RESOURCE->DATA = VertexShader;
			EDITOR_FILESYSTEM->Add_anAsset_toAssetList(RESOURCE);
			string compilation_status;
			//Add_anAsset_toAssetList gave an ID, so we can compile it now!
			GFXContentManager->Compile_ShaderSource(VertexShader, RESOURCE->ID, &compilation_status);
			VSSOURCE_ID = RESOURCE->ID;
		}

		//Load Fragment Shader
		unsigned int FSSOURCE_ID = 0;
		{
			LOG_STATUS("Loading Fragment Shader!\n");
			string* FRAGMENTSHADER_SOURCE = TAPIFILESYSTEM::Read_TextFile("C:/dev/StajRenderer/Content/SurfaceUberShader.frag");
			ShaderSource_Resource* FragmentShader = new ShaderSource_Resource;
			FragmentShader->LANGUAGE = GFX_API::SHADER_LANGUAGEs::GLSL;
			FragmentShader->STAGE = GFX_API::SHADER_STAGE::FRAGMENTSTAGE;
			FragmentShader->SOURCE_CODE = *FRAGMENTSHADER_SOURCE;

			Resource_Identifier* RESOURCE = new Resource_Identifier;
			RESOURCE->TYPE = RESOURCETYPEs::GFXAPI_SHADERSOURCE;
			RESOURCE->DATA = FragmentShader;
			EDITOR_FILESYSTEM->Add_anAsset_toAssetList(RESOURCE);
			string compilation_status;
			//Add_anAsset_toAssetList gave an ID, so we can compile it now!
			GFXContentManager->Compile_ShaderSource(FragmentShader, RESOURCE->ID, &compilation_status);
			FSSOURCE_ID = RESOURCE->ID;
		}


		Surface_MatType->VERTEXSOURCE_ID = VSSOURCE_ID;
		Surface_MatType->FRAGMENTSOURCE_ID = FSSOURCE_ID;

		//Resource Compilation
		Resource_Identifier* RESOURCE = new Resource_Identifier;
		RESOURCE->TYPE = RESOURCETYPEs::GFXAPI_MATTYPE;
		RESOURCE->DATA = Surface_MatType;
		EDITOR_FILESYSTEM->Add_anAsset_toAssetList(RESOURCE);
		string compilation_status;		//I don't care, because it will be compiled anyway!
		//Add_anAsset_toAssetList gave an ID to the Material Type, link it now!
		GFXContentManager->Link_MaterialType(Surface_MatType, RESOURCE->ID, &compilation_status);
		SurfaceMatType_ID = RESOURCE->ID;
	}
	void Editor_RendererDataManager::Load_PointLineMaterialType() {
		Material_Type* PointLineMaterial = new Material_Type;

		//OBJECT_INDEX uniform
		{
			Material_Uniform Object_Index;
			Object_Index.VARIABLE_NAME = "OBJECT_INDEX";
			Object_Index.VARIABLE_TYPE = GFX_API::DATA_TYPE::VAR_UINT32;
			Object_Index.DATA = nullptr;
			PointLineMaterial->UNIFORMs.push_back(Object_Index);
		}
		//POINTLIGHT_INDEX uniform
		{
			Material_Uniform PointLightIndex;
			PointLightIndex.VARIABLE_NAME = "POINTLIGHT_INDEX";
			PointLightIndex.VARIABLE_TYPE = GFX_API::DATA_TYPE::VAR_UINT32;
			PointLightIndex.DATA = nullptr;
			PointLineMaterial->UNIFORMs.push_back(PointLightIndex);
		}
		//SPOTLIGHT_INDEX uniform
		{
			Material_Uniform SpotLightIndex;
			SpotLightIndex.VARIABLE_NAME = "SPOTLIGHT_INDEX";
			SpotLightIndex.VARIABLE_TYPE = GFX_API::DATA_TYPE::VAR_UINT32;
			SpotLightIndex.DATA = nullptr;
			PointLineMaterial->UNIFORMs.push_back(SpotLightIndex);
		}
		//POINTCOLOR uniform
		{
			Material_Uniform POINTCOLOR;
			POINTCOLOR.VARIABLE_NAME = "POINTCOLOR";
			POINTCOLOR.VARIABLE_TYPE = GFX_API::DATA_TYPE::VAR_VEC3;
			POINTCOLOR.DATA = nullptr;
			PointLineMaterial->UNIFORMs.push_back(POINTCOLOR);
		}
		//POINT_SIZE uniform
		{
			Material_Uniform POINTSIZE;
			POINTSIZE.VARIABLE_NAME = "POINT_SIZE";
			POINTSIZE.VARIABLE_TYPE = GFX_API::DATA_TYPE::VAR_FLOAT32;
			POINTSIZE.DATA = nullptr;
			PointLineMaterial->UNIFORMs.push_back(POINTSIZE);
		}

		//GLOBAL BUFFER BINDING
		{
			GlobalBuffer_Access WORLDOBJECTs_ACCESS;
			WORLDOBJECTs_ACCESS.ACCESS_TYPE = OPERATION_TYPE::READ_ONLY;
			WORLDOBJECTs_ACCESS.BUFFER_ID = WORLDOBJECTs_GLOBALBUFFERID;
			PointLineMaterial->GLOBALBUFFERs.push_back(WORLDOBJECTs_ACCESS);

			GlobalBuffer_Access CAMERA_ACCESS;
			CAMERA_ACCESS.ACCESS_TYPE = OPERATION_TYPE::READ_ONLY;
			CAMERA_ACCESS.BUFFER_ID = CAMERA_GLOBALBUFFERID;
			PointLineMaterial->GLOBALBUFFERs.push_back(CAMERA_ACCESS);

			GlobalBuffer_Access LIGTs_ACCESS;
			LIGTs_ACCESS.ACCESS_TYPE = OPERATION_TYPE::READ_ONLY;
			LIGTs_ACCESS.BUFFER_ID = LIGHTs_GLOBALBUFFERID;
			PointLineMaterial->GLOBALBUFFERs.push_back(LIGTs_ACCESS);
		}

		//Load Vertex Shader
		unsigned int VSSOURCE_ID = 0;
		{
			LOG_STATUS("Loading Vertex Shader!\n");
			string* VERTEXSHADER_SOURCE = TAPIFILESYSTEM::Read_TextFile("C:/dev/StajRenderer/Content/PointRendererShader.vert");
			ShaderSource_Resource* VertexShader = new ShaderSource_Resource;
			VertexShader->LANGUAGE = GFX_API::SHADER_LANGUAGEs::GLSL;
			VertexShader->STAGE = GFX_API::SHADER_STAGE::VERTEXSTAGE;
			VertexShader->SOURCE_CODE = *VERTEXSHADER_SOURCE;

			Resource_Identifier* RESOURCE = new Resource_Identifier;
			RESOURCE->TYPE = RESOURCETYPEs::GFXAPI_SHADERSOURCE;
			RESOURCE->DATA = VertexShader;
			EDITOR_FILESYSTEM->Add_anAsset_toAssetList(RESOURCE);
			string compilation_status;
			//Add_anAsset_toAssetList gave an ID, so we can compile it now!
			GFXContentManager->Compile_ShaderSource(VertexShader, RESOURCE->ID, &compilation_status);
			VSSOURCE_ID = RESOURCE->ID;
		}

		//Load Fragment Shader
		unsigned int FSSOURCE_ID = 0;
		{
			LOG_STATUS("Loading Fragment Shader!\n");
			string* FRAGMENTSHADER_SOURCE = TAPIFILESYSTEM::Read_TextFile("C:/dev/StajRenderer/Content/PointRendererShader.frag");
			ShaderSource_Resource* FragmentShader = new ShaderSource_Resource;
			FragmentShader->LANGUAGE = GFX_API::SHADER_LANGUAGEs::GLSL;
			FragmentShader->STAGE = GFX_API::SHADER_STAGE::FRAGMENTSTAGE;
			FragmentShader->SOURCE_CODE = *FRAGMENTSHADER_SOURCE;

			Resource_Identifier* RESOURCE = new Resource_Identifier;
			RESOURCE->TYPE = RESOURCETYPEs::GFXAPI_SHADERSOURCE;
			RESOURCE->DATA = FragmentShader;
			EDITOR_FILESYSTEM->Add_anAsset_toAssetList(RESOURCE);
			string compilation_status;
			//Add_anAsset_toAssetList gave an ID, so we can compile it now!
			GFXContentManager->Compile_ShaderSource(FragmentShader, RESOURCE->ID, &compilation_status);
			FSSOURCE_ID = RESOURCE->ID;
		}

		PointLineMaterial->VERTEXSOURCE_ID = VSSOURCE_ID;
		PointLineMaterial->FRAGMENTSOURCE_ID = FSSOURCE_ID;

		//Resource Compilation
		Resource_Identifier* RESOURCE = new Resource_Identifier;
		RESOURCE->TYPE = RESOURCETYPEs::GFXAPI_MATTYPE;
		RESOURCE->DATA = PointLineMaterial;
		EDITOR_FILESYSTEM->Add_anAsset_toAssetList(RESOURCE);
		string compilation_status;		//I don't care, because it will be compiled anyway!
		//Add_anAsset_toAssetList gave an ID to the Material Type, link it now!
		GFXContentManager->Link_MaterialType(PointLineMaterial, RESOURCE->ID, &compilation_status);
		PointLineMaterialID = RESOURCE->ID;
	}

	unsigned int Editor_RendererDataManager::Create_SurfaceMaterialInstance(SURFACEMAT_PROPERTIES MaterialInstance_Properties, unsigned int* OBJECT_WORLDID) {
		Material_Instance* MATINST = new Material_Instance;
		MATINST->Material_Type = SurfaceMatType_ID;
		
		if (!OBJECT_WORLDID) {
			OBJECT_WORLDID = new unsigned int(Create_OBJECTINDEX());
		}
		//Uniform preparing
		{
			Material_Type* MATTYPE_RESOURCE = (Material_Type*)EDITOR_FILESYSTEM->Find_ResourceIdentifier_byID(SurfaceMatType_ID)->DATA;
			for (unsigned int i = 0; i < MATTYPE_RESOURCE->UNIFORMs.size(); i++) {
				Material_Uniform UNIFORM = MATTYPE_RESOURCE->UNIFORMs[i];
				MATINST->UNIFORM_LIST.push_back(UNIFORM);
			}
			MATINST->TEXTURE_LIST = MATTYPE_RESOURCE->TEXTUREs;
			for (unsigned int i = 0; i < MATINST->UNIFORM_LIST.size(); i++) {
				if (MATINST->UNIFORM_LIST[i].VARIABLE_NAME == "OBJECT_INDEX") {
					MATINST->UNIFORM_LIST[i].DATA = OBJECT_WORLDID;
				}
			}
			for (unsigned int i = 0; i < MATINST->TEXTURE_LIST.size(); i++) {
				if (MATINST->TEXTURE_LIST[i].ACCESS_TYPE != TEXTURE_ACCESS::SAMPLER_OPERATION) {
					continue;
				}
				//DIFFUSE
				if (MATINST->TEXTURE_LIST[i].BINDING_POINT == 0 && MaterialInstance_Properties.DIFFUSETEXTURE_ID) {
					Texture_Resource* TEXTURE_ASSET = (Texture_Resource*)EDITOR_FILESYSTEM->Find_ResourceIdentifier_byID(MaterialInstance_Properties.DIFFUSETEXTURE_ID)->DATA;
					GFXContentManager->Create_Texture(TEXTURE_ASSET, MaterialInstance_Properties.DIFFUSETEXTURE_ID);
					MATINST->TEXTURE_LIST[i].TEXTURE_ID = MaterialInstance_Properties.DIFFUSETEXTURE_ID;
				}
				//NORMALs
				else if (MATINST->TEXTURE_LIST[i].BINDING_POINT == 1 && MaterialInstance_Properties.NORMALSTEXTURE_ID) {
					Texture_Resource* TEXTURE_ASSET = (Texture_Resource*)EDITOR_FILESYSTEM->Find_ResourceIdentifier_byID(MaterialInstance_Properties.NORMALSTEXTURE_ID)->DATA;
					GFXContentManager->Create_Texture(TEXTURE_ASSET, MaterialInstance_Properties.NORMALSTEXTURE_ID);
					MATINST->TEXTURE_LIST[i].TEXTURE_ID = MaterialInstance_Properties.NORMALSTEXTURE_ID;
				}
				//SPECULAR
				else if (MATINST->TEXTURE_LIST[i].BINDING_POINT == 2 && MaterialInstance_Properties.SPECULARTEXTURE_ID) {
					Texture_Resource* TEXTURE_ASSET = (Texture_Resource*)EDITOR_FILESYSTEM->Find_ResourceIdentifier_byID(MaterialInstance_Properties.SPECULARTEXTURE_ID)->DATA;
					GFXContentManager->Create_Texture(TEXTURE_ASSET, MaterialInstance_Properties.SPECULARTEXTURE_ID);
					MATINST->TEXTURE_LIST[i].TEXTURE_ID = MaterialInstance_Properties.SPECULARTEXTURE_ID;
				}
				//OPACITY
				else if (MATINST->TEXTURE_LIST[i].BINDING_POINT == 3 && MaterialInstance_Properties.OPACITYTEXTURE_ID) {
					Texture_Resource* TEXTURE_ASSET = (Texture_Resource*)EDITOR_FILESYSTEM->Find_ResourceIdentifier_byID(MaterialInstance_Properties.OPACITYTEXTURE_ID)->DATA;
					GFXContentManager->Create_Texture(TEXTURE_ASSET, MaterialInstance_Properties.OPACITYTEXTURE_ID);
					MATINST->TEXTURE_LIST[i].TEXTURE_ID = MaterialInstance_Properties.OPACITYTEXTURE_ID;
				}
			}
		}

		Resource_Identifier* RESOURCE = new Resource_Identifier;
		RESOURCE->TYPE = RESOURCETYPEs::GFXAPI_MATINST;
		RESOURCE->DATA = MATINST;
		EDITOR_FILESYSTEM->Add_anAsset_toAssetList(RESOURCE);
		return RESOURCE->ID;
	}

	void Editor_RendererDataManager::Update_GPUResources() {
		//Camera Global Buffer Update
		{
			mat4 proj_mat;
			proj_mat = perspective(radians(45.0f), float(1920.0f / 1080.0f), 0.001f, 10000.0f);
			mat4 view_mat;
			view_mat = lookAt(CameraPos, FrontVec + CameraPos, vec3(0, 1, 0));
			CAMERABUFFER_DATA[0] = proj_mat;
			CAMERABUFFER_DATA[1] = view_mat;
			GFXContentManager->Upload_GlobalBuffer(CAMERA_GLOBALBUFFERID);
		}
		//World Objects Matrices Global Buffer Update
		{
			WORLDOBJECTs_BUFFERDATA[0] = {};
			WORLDOBJECTs_BUFFERDATA[0] = glm::translate(WORLDOBJECTs_BUFFERDATA[0], FirstObjectPosition);
			WORLDOBJECTs_BUFFERDATA[0] = glm::rotate(WORLDOBJECTs_BUFFERDATA[0], radians(FirstObjectRotation.x), vec3(1, 0, 0));
			WORLDOBJECTs_BUFFERDATA[0] = glm::rotate(WORLDOBJECTs_BUFFERDATA[0], radians(FirstObjectRotation.y), vec3(0, 1, 0));
			WORLDOBJECTs_BUFFERDATA[0] = glm::rotate(WORLDOBJECTs_BUFFERDATA[0], radians(FirstObjectRotation.z), vec3(0, 0, 1));
			GFXContentManager->Upload_GlobalBuffer(WORLDOBJECTs_GLOBALBUFFERID);
		}
		//Lights Global Buffer Update
		{
			memcpy(LIGHTsBUFFER_DATA, DIRECTIONALLIGHTs, 32);
			memcpy((char*)LIGHTsBUFFER_DATA + 32, POINTLIGHTs, 160);
			memcpy((char*)LIGHTsBUFFER_DATA + 192, SPOTLIGHTs, 160);
			memcpy((char*)LIGHTsBUFFER_DATA + 352, &DIRECTIONALLIGHTs_COUNT, 4);
			memcpy((char*)LIGHTsBUFFER_DATA + 356, &POINTLIGHTs_COUNT, 4);
			memcpy((char*)LIGHTsBUFFER_DATA + 360, &SPOTLIGHTs_COUNT, 4);
			GFXContentManager->Upload_GlobalBuffer(LIGHTs_GLOBALBUFFERID);
		}
	}

	void Editor_RendererDataManager::UpdateGeodesicDistances(const void* DATA, unsigned int DATASIZE) {
		GFXContentManager->Upload_GlobalBuffer(GeodesicDistancesBuffer_ID, DATA, DATASIZE);
	}

	void Editor_RendererDataManager::Start_RenderingDataManagement() {
		Load_SurfaceMaterialType();
		Load_PointLineMaterialType();
	}

	//0 is valid!
	unsigned int Editor_RendererDataManager::Create_OBJECTINDEX() {
		unsigned int index = WORLDOBJECT_IDs.GetIndex_FirstFalse();
		WORLDOBJECT_IDs.SetBit_True(index);
		return index;
	}
	void Editor_RendererDataManager::Delete_OBJECTINDEX(unsigned int INDEX) {
		WORLDOBJECT_IDs.SetBit_False(INDEX);
	}


}