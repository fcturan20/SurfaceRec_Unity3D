#include "Editor_DataManager.h"
#include "Editor/FileSystem/EditorFileSystem_Core.h"
//To show import status
#include "Editor/Editors/Status_Window.h"
//To compile shader program!
#include "GFX/GFX_Core.h"
#include "Editor/FileSystem/ResourceTypes/ResourceTYPEs.h"
#include "Editor\FileSystem\ResourceImporters\Model_Loader.h"
#define LIGHTSBUFFER_DATASIZE 32

using namespace GFX_API;

static TuranEditor::Static_Model* DefaultSphereModel = nullptr;
static Game_RenderGraph* RG = nullptr;

namespace TuranEditor {
	//Point Renderer
	static unsigned int PointMatInst_NonShading = UINT32_MAX, PointMatInst_Shading = UINT32_MAX, PointMatTypeID = UINT32_MAX;
	static GFX_API::VertexAttributeLayout PointRenderer_VertexAttrib;
	vector<POINTRENDERER*> RenderDataManager::PointRenderers;

	//Plane Renderer
	static unsigned int PlaneMatInst = UINT32_MAX, PlaneMatTypeID = UINT32_MAX;
	static GFX_API::VertexAttributeLayout PlaneRenderer_VertexAttrib;
	std::vector<vec3*> PlaneRenderer_UniformDataDeletion;


	GFX_API::VertexAttributeLayout RenderDataManager::PositionNormal_VertexAttrib, RenderDataManager::PositionOnly_VertexAttrib;
	static Bitset WORLDOBJECT_IDs(100);
	static unsigned int Create_OBJECTINDEX();
	static void Delete_OBJECTINDEX(unsigned int INDEX);
	unsigned int RenderDataManager::WORLDOBJECTs_GLOBALBUFFERID, RenderDataManager::MATINSTs_GLOBALBUFFERID, RenderDataManager::CAMERA_GLOBALBUFFERID, RenderDataManager::LIGHTs_GLOBALBUFFERID,
		RenderDataManager::SurfaceMatType_ID;
	Directional_Light* RenderDataManager::DIRECTIONALLIGHTs = new Directional_Light[MAX_DIRECTIONALLIGHTs];
	unsigned int RenderDataManager::DIRECTIONALLIGHTs_COUNT = 0;
	
	void* RenderDataManager::LIGHTsBUFFER_DATA = new bool[LIGHTSBUFFER_DATASIZE];
	vec3 RenderDataManager::CameraPos(0, 0, -3), RenderDataManager::FrontVec(0, 0, 1)
		, RenderDataManager::FirstObjectPosition(0,0,0), RenderDataManager::FirstObjectRotation(-90.0f, 0.0f, -90.0f);
	mat4* RenderDataManager::CAMERABUFFER_DATA = new mat4[2],
		*RenderDataManager::WORLDOBJECTs_BUFFERDATA = new mat4[MAX_WORLDOBJECTs];
	void Load_SurfaceMaterialType() {
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
		//isPhongShadingActive uniform
		{
			Material_Uniform uniform_isphongshadingactive;
			uniform_isphongshadingactive.VARIABLE_NAME = "isPhongShadingActive";
			uniform_isphongshadingactive.VARIABLE_TYPE = GFX_API::DATA_TYPE::VAR_UINT32;
			uniform_isphongshadingactive.DATA = nullptr;
			Surface_MatType->UNIFORMs.push_back(uniform_isphongshadingactive);
		}
		

		RenderDataManager::DIRECTIONALLIGHTs[0].COLOR = vec4(1, 0.5f, 0.2f, 0);
		RenderDataManager::DIRECTIONALLIGHTs[0].DIRECTION = vec4(0.5f, 0.7f, 0.9f, 0);

		//Global Buffers

		//Camera Buffer
		{
			mat4 proj_mat;
			proj_mat = perspective(radians(45.0f), float(1920.0f / 1080.0f), 0.1f, 10000.0f);
			mat4 view_mat;
			view_mat = glm::lookAt(RenderDataManager::CameraPos, RenderDataManager::FrontVec + RenderDataManager::CameraPos, vec3(0, 1, 0));
			RenderDataManager::CAMERABUFFER_DATA[0] = proj_mat;
			RenderDataManager::CAMERABUFFER_DATA[1] = view_mat;
			RenderDataManager::CAMERA_GLOBALBUFFERID = GFXContentManager->Create_GlobalBuffer("VIEW_MATRICES", RenderDataManager::CAMERABUFFER_DATA, 128, GFX_API::BUFFER_VISIBILITY::CPUREADWRITE_GPUREADWRITE);
		}
		//World Objects Matrices Buffer
		{
			RenderDataManager::WORLDOBJECTs_GLOBALBUFFERID = GFXContentManager->Create_GlobalBuffer("MODEL_MATRICES", RenderDataManager::WORLDOBJECTs_BUFFERDATA, 16000, GFX_API::BUFFER_VISIBILITY::CPUREADWRITE_GPUREADWRITE);
		}
		//Lights Buffer
		{
			memcpy((char*)RenderDataManager::LIGHTsBUFFER_DATA, RenderDataManager::DIRECTIONALLIGHTs, 32);
			RenderDataManager::LIGHTs_GLOBALBUFFERID = GFXContentManager->Create_GlobalBuffer("LIGHTs", RenderDataManager::LIGHTsBUFFER_DATA, LIGHTSBUFFER_DATASIZE, GFX_API::BUFFER_VISIBILITY::CPUREADWRITE_GPUREADWRITE);
		}

		//GLOBAL BUFFER BINDING
		{
			GlobalBuffer_Access WORLDOBJECTs_ACCESS;
			WORLDOBJECTs_ACCESS.ACCESS_TYPE = OPERATION_TYPE::READ_ONLY;
			WORLDOBJECTs_ACCESS.BUFFER_ID = RenderDataManager::WORLDOBJECTs_GLOBALBUFFERID;
			Surface_MatType->GLOBALBUFFERs.push_back(WORLDOBJECTs_ACCESS);

			GlobalBuffer_Access CAMERA_ACCESS;
			CAMERA_ACCESS.ACCESS_TYPE = OPERATION_TYPE::READ_ONLY;
			CAMERA_ACCESS.BUFFER_ID = RenderDataManager::CAMERA_GLOBALBUFFERID;
			Surface_MatType->GLOBALBUFFERs.push_back(CAMERA_ACCESS);

			GlobalBuffer_Access LIGHTs_ACCESS;
			LIGHTs_ACCESS.ACCESS_TYPE = OPERATION_TYPE::READ_ONLY;
			LIGHTs_ACCESS.BUFFER_ID = RenderDataManager::LIGHTs_GLOBALBUFFERID;
			Surface_MatType->GLOBALBUFFERs.push_back(LIGHTs_ACCESS);
		}


		//Load Vertex Shader
		unsigned int VSSOURCE_ID = 0;
		{
			LOG_STATUS("Loading Vertex Shader!\n");
			string* VERTEXSHADER_SOURCE = TAPIFILESYSTEM::Read_TextFile("D:/dev/GeometryProcessing/Content/SurfaceUberShader.vert");
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
			string* FRAGMENTSHADER_SOURCE = TAPIFILESYSTEM::Read_TextFile("D:/dev/GeometryProcessing/Content/SurfaceUberShader.frag");
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
		RenderDataManager::SurfaceMatType_ID = RESOURCE->ID;
	}
	void Load_PointLineMaterialType() {
		Material_Type* PointLineMaterial = new Material_Type;

		//PhongShading uniform
		{
			Material_Uniform phong_active;
			phong_active.VARIABLE_NAME = "isPhongShadingActive";
			phong_active.VARIABLE_TYPE = GFX_API::DATA_TYPE::VAR_UINT32;
			phong_active.DATA = nullptr;
			PointLineMaterial->UNIFORMs.push_back(phong_active);
		}

		//GLOBAL BUFFER BINDING
		{
			GlobalBuffer_Access CAMERA_ACCESS;
			CAMERA_ACCESS.ACCESS_TYPE = OPERATION_TYPE::READ_ONLY;
			CAMERA_ACCESS.BUFFER_ID = RenderDataManager::CAMERA_GLOBALBUFFERID;
			PointLineMaterial->GLOBALBUFFERs.push_back(CAMERA_ACCESS);

			GlobalBuffer_Access LIGHTs_ACCESS;
			LIGHTs_ACCESS.ACCESS_TYPE = OPERATION_TYPE::READ_ONLY;
			LIGHTs_ACCESS.BUFFER_ID = RenderDataManager::LIGHTs_GLOBALBUFFERID;
			PointLineMaterial->GLOBALBUFFERs.push_back(LIGHTs_ACCESS);
		}

		//Load Vertex Shader
		unsigned int VSSOURCE_ID = 0;
		{
			LOG_STATUS("Loading Vertex Shader!\n");
			string* VERTEXSHADER_SOURCE = TAPIFILESYSTEM::Read_TextFile("D:/dev/GeometryProcessing/Content/PointRendererShader.vert");
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
		/*
		//Load Geometry Shader
		unsigned int GSSOURCE_ID = 0;
		{
			LOG_STATUS("Loading Geometry Shader!\n");
			string* GEOMETRYSHADER_SOURCE = TAPIFILESYSTEM::Read_TextFile("D:/dev/GeometryProcessing/Content/PointRendererShader.gs");
			ShaderSource_Resource* GeometryShader = new ShaderSource_Resource;
			GeometryShader->LANGUAGE = GFX_API::SHADER_LANGUAGEs::GLSL;
			GeometryShader->STAGE = GFX_API::SHADER_STAGE::GEOMETRYSTAGE;
			GeometryShader->SOURCE_CODE = *GEOMETRYSHADER_SOURCE;

			Resource_Identifier* RESOURCE = new Resource_Identifier;
			RESOURCE->TYPE = RESOURCETYPEs::GFXAPI_SHADERSOURCE;
			RESOURCE->DATA = GeometryShader;
			EDITOR_FILESYSTEM->Add_anAsset_toAssetList(RESOURCE);
			string compilation_status;
			//Add_anAsset_toAssetList gave an ID, so we can compile it now!
			GFXContentManager->Compile_ShaderSource(GeometryShader, RESOURCE->ID, &compilation_status);
			GSSOURCE_ID = RESOURCE->ID;
		}*/

		//Load Fragment Shader
		unsigned int FSSOURCE_ID = 0;
		{
			LOG_STATUS("Loading Fragment Shader!\n");
			string* FRAGMENTSHADER_SOURCE = TAPIFILESYSTEM::Read_TextFile("D:/dev/GeometryProcessing/Content/PointRendererShader.frag");
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
		//PointLineMaterial->GEOMETRYSOURCE_ID = GSSOURCE_ID;
		PointLineMaterial->FRAGMENTSOURCE_ID = FSSOURCE_ID;

		//Resource Compilation
		Resource_Identifier* RESOURCE = new Resource_Identifier;
		RESOURCE->TYPE = RESOURCETYPEs::GFXAPI_MATTYPE;
		RESOURCE->DATA = PointLineMaterial;
		EDITOR_FILESYSTEM->Add_anAsset_toAssetList(RESOURCE);
		string compilation_status;		//I don't care, because it will be compiled anyway!
		//Add_anAsset_toAssetList gave an ID to the Material Type, link it now!
		GFXContentManager->Link_MaterialType(PointLineMaterial, RESOURCE->ID, &compilation_status);
		PointMatTypeID = RESOURCE->ID;
	}

	unsigned int RenderDataManager::Create_SurfaceMaterialInstance(SURFACEMAT_PROPERTIES MaterialInstance_Properties, unsigned int* OBJECT_WORLDID, unsigned int isUsingPhongShading) {
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
				if (MATINST->UNIFORM_LIST[i].VARIABLE_NAME == "isPhongShadingActive") {
					MATINST->UNIFORM_LIST[i].DATA = new unsigned int(isUsingPhongShading);
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
	vector<vec3> RenderDataManager::Create_Sphere(glm::vec3 Center, float radius) {
		vector<vec3> SphereVertexes(DefaultSphereModel->MESHes[0]->INDICES_NUMBER, vec3(0.0));
		for (unsigned int i = 0; i < SphereVertexes.size(); i++) {
			SphereVertexes[i] = Center + (((vec3*)DefaultSphereModel->MESHes[0]->VERTEX_DATA)[DefaultSphereModel->MESHes[0]->INDEX_DATA[i]] * vec3(radius));
		}
		return SphereVertexes;
	}

	unsigned int RenderDataManager::Create_Plane(glm::vec3 Center, glm::vec3 Tangent, glm::vec3 Bitangent, glm::vec2 Size) {
		glm::vec3 PlaneVertexes[6];
		PlaneVertexes[0] = Center + (Bitangent * glm::vec3(Size.x / 2.0f)) - (Tangent * glm::vec3(Size.y / 2.0f));
		PlaneVertexes[1] = Center + (Bitangent * glm::vec3(Size.x / 2.0f)) + (Tangent * glm::vec3(Size.y / 2.0f));
		PlaneVertexes[2] = Center - (Bitangent * glm::vec3(Size.x / 2.0f)) + (Tangent * glm::vec3(Size.y / 2.0f));
		PlaneVertexes[3] = Center - (Bitangent * glm::vec3(Size.x / 2.0f)) + (Tangent * glm::vec3(Size.y / 2.0f));
		PlaneVertexes[4] = Center + (Bitangent * glm::vec3(Size.x / 2.0f)) - (Tangent * glm::vec3(Size.y / 2.0f));
		PlaneVertexes[5] = Center - (Bitangent * glm::vec3(Size.x / 2.0f)) - (Tangent * glm::vec3(Size.y / 2.0f));
		return GFXContentManager->Upload_MeshBuffer(PositionOnly_VertexAttrib, &PlaneVertexes, 12 * 6, 6, nullptr, 0);
	}


	void RenderDataManager::Update_GPUResources() {
		//Update Global Buffers
		{
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
				DIRECTIONALLIGHTs[0].DIRECTION = normalize(DIRECTIONALLIGHTs[0].DIRECTION);
				memcpy((char*)LIGHTsBUFFER_DATA, DIRECTIONALLIGHTs, 32);
				GFXContentManager->Upload_GlobalBuffer(LIGHTs_GLOBALBUFFERID);
			}
		}

		//Update PointRenderer Buffers
		for (unsigned int i = 0; i < PointRenderers.size(); i++) {
			if (PointRenderers[i]->isResourcesUpdated) {
				bool* data = new bool[PointRenderers[i]->PositionXYZs.size() * 28];
				memcpy(data, PointRenderers[i]->PositionXYZs.data(), PointRenderers[i]->PositionXYZs.size() * 12);
				memcpy(data + (PointRenderers[i]->PositionXYZs.size() * 12), PointRenderers[i]->PointCOLORRGBAs.data(), PointRenderers[i]->PositionXYZs.size() * 16);
				GFXContentManager->Upload_PointBuffer(PointRenderers[i]->GPUHandle, data);
				delete[] data;
				PointRenderers[i]->isResourcesUpdated = false;
			}
		}


		//Draw calls of active PointRenderers
		for(unsigned int i = 0; i < PointRenderers.size(); i++){
			if (PointRenderers[i]->shouldRenderThisFrame) {
				PointLineDrawCall dc;
				dc.Draw_asPoint = true;
				dc.PointBuffer_ID = PointRenderers[i]->GPUHandle;
				dc.ShaderInstance_ID = PointRenderers[i]->isPhongShadingActive ? PointMatInst_Shading : PointMatInst_NonShading;
				PointRenderers[i]->shouldRenderThisFrame = false;
				RG->Register_PointDrawCall(dc);
			}
		}
		

	}


	//Point Cloud, Surface Reconstruction and Normal Estimation Study
	void CreateVertexAttributeLayouts() {
		{
			RenderDataManager::PositionNormal_VertexAttrib.Attributes.push_back(GFX_API::VertexAttribute());
			GFX_API::VertexAttribute& PosAttrib = RenderDataManager::PositionNormal_VertexAttrib.Attributes[0];
			PosAttrib.AttributeName = "Positions";
			PosAttrib.DATATYPE = GFX_API::DATA_TYPE::VAR_VEC3;
			PosAttrib.Index = 0;
			PosAttrib.Start_Offset = 0;
			PosAttrib.Stride = 0;

			RenderDataManager::PositionNormal_VertexAttrib.Attributes.push_back(GFX_API::VertexAttribute());
			GFX_API::VertexAttribute& NorAttrib = RenderDataManager::PositionNormal_VertexAttrib.Attributes[1];
			NorAttrib.AttributeName = "Normals";
			NorAttrib.DATATYPE = GFX_API::DATA_TYPE::VAR_VEC3;
			NorAttrib.Index = 1;
			NorAttrib.Start_Offset = 0;
			NorAttrib.Stride = 0;

			RenderDataManager::PositionOnly_VertexAttrib.Attributes.push_back(GFX_API::VertexAttribute());
			GFX_API::VertexAttribute& PosAttrib2 = RenderDataManager::PositionOnly_VertexAttrib.Attributes[0];
			PosAttrib2.AttributeName = "Positions";
			PosAttrib2.DATATYPE = GFX_API::DATA_TYPE::VAR_VEC3;
			PosAttrib2.Index = 0;
			PosAttrib2.Start_Offset = 0;
			PosAttrib2.Stride = 3;
		}
		RenderDataManager::PositionNormal_VertexAttrib.Calculate_SizeperVertex();
		RenderDataManager::PositionOnly_VertexAttrib.Calculate_SizeperVertex();

		//Point Renderer Vertex Attribute Layout
		{
			PointRenderer_VertexAttrib.Attributes.push_back(GFX_API::VertexAttribute());
			GFX_API::VertexAttribute& PosAttrib = PointRenderer_VertexAttrib.Attributes[0];
			PosAttrib.AttributeName = "Point_POS";
			PosAttrib.DATATYPE = GFX_API::DATA_TYPE::VAR_VEC3;
			PosAttrib.Index = 0;
			PosAttrib.Start_Offset = 0;
			PosAttrib.Stride = 3;

			PointRenderer_VertexAttrib.Attributes.push_back(GFX_API::VertexAttribute());
			GFX_API::VertexAttribute& ColorAttrib = PointRenderer_VertexAttrib.Attributes[1];
			ColorAttrib.AttributeName = "Point_COLOR";
			ColorAttrib.DATATYPE = GFX_API::DATA_TYPE::VAR_VEC4;
			ColorAttrib.Index = 1;
			ColorAttrib.Start_Offset = 0;
			ColorAttrib.Stride = 4;
		}
		PointRenderer_VertexAttrib.Calculate_SizeperVertex();
	}
	void CreateGeometryProcessingMaterials() {
		//Create PointMatInst_NonShading
		{
			GFX_API::Material_Instance* PointMatInstRED = new GFX_API::Material_Instance;
			PointMatInstRED->Material_Type = PointMatTypeID;

			//Uniform Preparing
			GFX_API::Material_Type* POINTLINE_MATTYPE = (GFX_API::Material_Type*)TuranEditor::EDITOR_FILESYSTEM->Find_ResourceIdentifier_byID(PointMatTypeID)->DATA;
			PointMatInstRED->UNIFORM_LIST = POINTLINE_MATTYPE->UNIFORMs;
			for (unsigned int i = 0; i < PointMatInstRED->UNIFORM_LIST.size(); i++) {
				if (PointMatInstRED->UNIFORM_LIST[i].VARIABLE_NAME == "isPhongShadingActive") {
					PointMatInstRED->UNIFORM_LIST[i].DATA = new unsigned int(false);
				}
			}
			TuranEditor::Resource_Identifier* LineMatInstResource = new TuranEditor::Resource_Identifier;
			LineMatInstResource->TYPE = TuranEditor::RESOURCETYPEs::GFXAPI_MATINST;
			LineMatInstResource->DATA = PointMatInstRED;
			TuranEditor::EDITOR_FILESYSTEM->Add_anAsset_toAssetList(LineMatInstResource);
			PointMatInst_NonShading = LineMatInstResource->ID;
		}
		//Create PointMatInst_Shading
		{
			GFX_API::Material_Instance* PointMatInst_shading = new GFX_API::Material_Instance;
			PointMatInst_shading->Material_Type = PointMatTypeID;

			//Uniform Preparing
			GFX_API::Material_Type* POINTLINE_MATTYPE = (GFX_API::Material_Type*)TuranEditor::EDITOR_FILESYSTEM->Find_ResourceIdentifier_byID(PointMatTypeID)->DATA;
			PointMatInst_shading->UNIFORM_LIST = POINTLINE_MATTYPE->UNIFORMs;
			for (unsigned int i = 0; i < PointMatInst_shading->UNIFORM_LIST.size(); i++) {
				if (PointMatInst_shading->UNIFORM_LIST[i].VARIABLE_NAME == "isPhongShadingActive") {
					PointMatInst_shading->UNIFORM_LIST[i].DATA = new unsigned int(true);
				}
			}
			TuranEditor::Resource_Identifier* LineMatInstResource = new TuranEditor::Resource_Identifier;
			LineMatInstResource->TYPE = TuranEditor::RESOURCETYPEs::GFXAPI_MATINST;
			LineMatInstResource->DATA = PointMatInst_shading;
			TuranEditor::EDITOR_FILESYSTEM->Add_anAsset_toAssetList(LineMatInstResource);
			PointMatInst_Shading = LineMatInstResource->ID;
		}

		//Create PlaneMatType and PlaneMatInst to use in PlaneSpecialDrawCalls
		{
			GFX_API::Material_Type* PlaneMatTYPE = new GFX_API::Material_Type;

			//PlaneColor uniform
			{
				Material_Uniform PlaneColor;
				PlaneColor.VARIABLE_NAME = "PlaneColor";
				PlaneColor.VARIABLE_TYPE = GFX_API::DATA_TYPE::VAR_VEC3;
				PlaneColor.DATA = nullptr;
				PlaneMatTYPE->UNIFORMs.push_back(PlaneColor);

				Material_Uniform Vertex0;
				PlaneColor.VARIABLE_NAME = "Vertex0";
				PlaneColor.VARIABLE_TYPE = GFX_API::DATA_TYPE::VAR_VEC3;
				PlaneColor.DATA = nullptr;
				PlaneMatTYPE->UNIFORMs.push_back(Vertex0);

				Material_Uniform Vertex1;
				PlaneColor.VARIABLE_NAME = "Vertex1";
				PlaneColor.VARIABLE_TYPE = GFX_API::DATA_TYPE::VAR_VEC3;
				PlaneColor.DATA = nullptr;
				PlaneMatTYPE->UNIFORMs.push_back(Vertex1);

				Material_Uniform Vertex2;
				PlaneColor.VARIABLE_NAME = "Vertex2";
				PlaneColor.VARIABLE_TYPE = GFX_API::DATA_TYPE::VAR_VEC3;
				PlaneColor.DATA = nullptr;
				PlaneMatTYPE->UNIFORMs.push_back(Vertex2);

				Material_Uniform Vertex3;
				PlaneColor.VARIABLE_NAME = "Vertex3";
				PlaneColor.VARIABLE_TYPE = GFX_API::DATA_TYPE::VAR_VEC3;
				PlaneColor.DATA = nullptr;
				PlaneMatTYPE->UNIFORMs.push_back(Vertex3);

				Material_Uniform Vertex4;
				PlaneColor.VARIABLE_NAME = "Vertex4";
				PlaneColor.VARIABLE_TYPE = GFX_API::DATA_TYPE::VAR_VEC3;
				PlaneColor.DATA = nullptr;
				PlaneMatTYPE->UNIFORMs.push_back(Vertex4);

				Material_Uniform Vertex5;
				PlaneColor.VARIABLE_NAME = "Vertex5";
				PlaneColor.VARIABLE_TYPE = GFX_API::DATA_TYPE::VAR_VEC3;
				PlaneColor.DATA = nullptr;
				PlaneMatTYPE->UNIFORMs.push_back(Vertex5);
			}

			//Load Vertex Shader
			unsigned int VSSOURCE_ID = 0;
			{
				LOG_STATUS("Loading Vertex Shader!\n");
				string* VERTEXSHADER_SOURCE = TAPIFILESYSTEM::Read_TextFile("D:/dev/GeometryProcessing/Content/PlaneRendererShader.vert");
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
				string* FRAGMENTSHADER_SOURCE = TAPIFILESYSTEM::Read_TextFile("D:/dev/GeometryProcessing/Content/PlaneRendererShader.frag");
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

			PlaneMatTYPE->VERTEXSOURCE_ID = VSSOURCE_ID;
			//PointLineMaterial->GEOMETRYSOURCE_ID = GSSOURCE_ID;
			PlaneMatTYPE->FRAGMENTSOURCE_ID = FSSOURCE_ID;

			//Resource Compilation
			Resource_Identifier* RESOURCE = new Resource_Identifier;
			RESOURCE->TYPE = RESOURCETYPEs::GFXAPI_MATTYPE;
			RESOURCE->DATA = PlaneMatTYPE;
			EDITOR_FILESYSTEM->Add_anAsset_toAssetList(RESOURCE);
			string compilation_status;		//I don't care, because it will be compiled anyway!
			//Add_anAsset_toAssetList gave an ID to the Material Type, link it now!
			GFXContentManager->Link_MaterialType(PlaneMatTYPE, RESOURCE->ID, &compilation_status);
			PlaneMatTypeID = RESOURCE->ID;



			GFX_API::Material_Instance* PlaneMatINST = new GFX_API::Material_Instance;
			PlaneMatINST->Material_Type = PlaneMatInst;
			
			//Uniform Preparing
			PlaneMatINST->UNIFORM_LIST = PlaneMatTYPE->UNIFORMs;
			TuranEditor::Resource_Identifier* PlaneMatINSTRESOURCE = new TuranEditor::Resource_Identifier;
			PlaneMatINSTRESOURCE->TYPE = TuranEditor::RESOURCETYPEs::GFXAPI_SHADERSOURCE;
			PlaneMatINSTRESOURCE->DATA = PlaneMatINST;
			TuranEditor::EDITOR_FILESYSTEM->Add_anAsset_toAssetList(PlaneMatINSTRESOURCE);
			PlaneMatInst = PlaneMatINSTRESOURCE->ID;
		}

	}
	POINTRENDERER* RenderDataManager::Create_PointRenderer(unsigned int PointCount) {
		PointRenderers.push_back(new POINTRENDERER(PointCount));
		return PointRenderers[PointRenderers.size() - 1];
	}
	void RenderDataManager::DestroyPointRenderer(POINTRENDERER* pointrenderer) {
		for (unsigned int i = 0; i < PointRenderers.size(); i++) {
			if (pointrenderer == PointRenderers[i]) {
				PointRenderers.erase(PointRenderers.begin() + i);
				delete pointrenderer;
			}
		}
	}
	GFX_API::SpecialDrawCall RenderDataManager::Create_PlaneSpecialDrawCall(vec3 Center, vec3 Tangent, vec3 Bitangent, vec2 Size, vec3 COLOR) {
		return GFX_API::SpecialDrawCall();
	}
	GFX_API::SpecialDrawCall RenderDataManager::Create_BoundingBoxSpecialDrawCall(vec3 BoundingMin, vec3 BoundingMax) {
		return GFX_API::SpecialDrawCall();
	}

	//Default Meshes
	void LoadDefaultMeshes() {
		DefaultSphereModel = (TuranEditor::Static_Model*)TuranEditor::EDITOR_FILESYSTEM->Find_ResourceIdentifier_byID(
			TuranEditor::Model_Importer::Import_Model("C:/Users/furka/Desktop/Meshes/Sphere_ManifoldMesh.obj"))->DATA;
	}

	void RenderDataManager::Start_RenderingDataManagement(Game_RenderGraph* DefaultRG) {
		RG = DefaultRG;
		Load_SurfaceMaterialType();
		Load_PointLineMaterialType();

		//Point Cloud, Surface Reconstruction and Normal Estimation Study
		CreateVertexAttributeLayouts();
		CreateGeometryProcessingMaterials();
		
		//Default Meshes
		LoadDefaultMeshes();
	}

	//0 is valid!
	unsigned int Create_OBJECTINDEX() {
		unsigned int index = WORLDOBJECT_IDs.GetIndex_FirstFalse();
		WORLDOBJECT_IDs.SetBit_True(index);
		return index;
	}
	void Delete_OBJECTINDEX(unsigned int INDEX) {
		WORLDOBJECT_IDs.SetBit_False(INDEX);
	}


	POINTRENDERER::POINTRENDERER(unsigned int PointCount) : PositionXYZs(PointCount), PointCOLORRGBAs(PointCount) {
		GPUHandle = GFXContentManager->Create_PointBuffer(PointRenderer_VertexAttrib, nullptr, PointCount);
	}
	vec3& POINTRENDERER::GetPointPosition_byIndex(unsigned int i) {
#ifdef _DEBUG
		if (i > PositionXYZs.size()) {
			LOG_CRASHING("Overflowing access!");
		}
#endif
		isResourcesUpdated = true;
		return PositionXYZs[i];
	}
	vec4& POINTRENDERER::GetPointCOLORRGBA_byIndex(unsigned int i) {
#ifdef _DEBUG
		if (i > PointCOLORRGBAs.size()) {
			LOG_CRASHING("Overflowing access!");
		}
#endif
		isResourcesUpdated = true;
		return PointCOLORRGBAs[i];
	}
	void POINTRENDERER::RenderThisFrame() {
		shouldRenderThisFrame = true;
	}
	POINTRENDERER::~POINTRENDERER() {
		shouldRenderThisFrame = false;
		isResourcesUpdated = false;
	}
}