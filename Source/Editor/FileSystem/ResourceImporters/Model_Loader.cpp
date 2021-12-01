#include "Model_Loader.h"
#include "Editor/FileSystem/EditorFileSystem_Core.h"

#include "Editor/Editors/Status_Window.h"

//Assimp libraries to load Model
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Editor/FileSystem/ResourceTypes/Model_Resource.h"
#include "Editor/FileSystem/ResourceTypes/ResourceTYPEs.h"
#include "GFX/GFX_Core.h"
#include "Editor/FileSystem/ResourceImporters/Texture_Loader.h"
#include "Editor/RenderContext/Editor_DataManager.h"

using namespace TuranAPI;

namespace TuranEditor {

	struct Attribute_BitMask {
		bool TextCoords : 1;
		bool Normal : 1;
		bool GeodesicDistance;
		Attribute_BitMask() {
			TextCoords = 0;
			Normal = false;
		}
	};
	void Find_AvailableAttributes(aiMesh* data, Attribute_BitMask& Available_Attributes);
	void Load_MeshData(const aiMesh* data, Attribute_BitMask& Attribute_Info, Static_Model* Model);
	unsigned int Load_MaterialData(const aiMaterial* data, const char* directory, unsigned int* object_worldid);

	unsigned int Model_Importer::Import_Model(const char* PATH, bool Use_SurfaceMaterial, unsigned int MaterialType_toInstance) {
		//Check if model is available
		Assimp::Importer import;
		const aiScene* Scene = nullptr;
		{
			Scene = import.ReadFile(PATH, aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_FindInvalidData
			);

			//Check if scene reading errors!
			if (!Scene || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Scene->mRootNode) {
				std::cout << (("Failed on Loading Mesh with Assimp; " + string(import.GetErrorString())).c_str()) << std::endl;
				return 0;
			}

			if (Scene->mNumMeshes == 0) {
				std::cout << "Failed because there is no mesh in loaded scene!"  << std::endl;
				return 0;
			}
		}

		//Importing process
		Static_Model* Loaded_Model = new Static_Model;
		{
			string compilation_status;


			vector<Attribute_BitMask> aiMesh_Attributes;
			aiMesh_Attributes.resize(Scene->mNumMeshes);
			for (unsigned int i = 0; i < Scene->mNumMeshes; i++) {
				Find_AvailableAttributes(Scene->mMeshes[i], aiMesh_Attributes[i]);
			}


			//Show Attribute Infos
			Attribute_BitMask* bitmask = nullptr;
			{
				for (unsigned int i = 0; i < Scene->mNumMeshes; i++) {
					bitmask = &aiMesh_Attributes[i];
					compilation_status.append("Mesh Index: ");
					compilation_status.append(std::to_string(i).c_str());
					compilation_status.append(" Texture Coordinate Sets Number: ");
					compilation_status.append(std::to_string(bitmask->TextCoords).c_str());
					if (bitmask->Normal) {
						compilation_status.append(" Vertex Normals're found\n");
					}
					else {
						compilation_status.append(" Vertex Normals're not found\n");
					}
				}
			}


			for (unsigned int i = 0; i < Scene->mNumMeshes; i++) {
				Load_MeshData(Scene->mMeshes[i], aiMesh_Attributes[i], Loaded_Model);
			}

			//Finalization
			compilation_status.append("Compiled the model successfully!");
		}

		//If model has materials, create a Folder and load materials etc.
		if (Scene->mNumMaterials) {
			if (Use_SurfaceMaterial) {
				string DIR = PATH;
				DIR = DIR.substr(0, DIR.find_last_of('/'));
				Loaded_Model->MATINST_IDs.resize(Scene->mNumMaterials);
				for (unsigned int i = 0; i < Loaded_Model->MATINST_IDs.size(); i++) {
					aiMaterial* MAT = Scene->mMaterials[i];
					Loaded_Model->MATINST_IDs[i] = Load_MaterialData(MAT, DIR.c_str(), Loaded_Model->WORLD_ID);
				}
			}
			else {
				LOG_NOTCODED("While loading a model, you have to use Surface Material Type for now!", true);
				return 0;
			}
		}

		if (Loaded_Model) {
			Resource_Identifier* RESOURCE = new Resource_Identifier;
			RESOURCE->PATH = PATH;
			RESOURCE->DATA = Loaded_Model;
			RESOURCE->TYPE = RESOURCETYPEs::EDITOR_STATICMODEL;
			TuranEditor::EDITOR_FILESYSTEM->Add_anAsset_toAssetList(RESOURCE);
			return RESOURCE->ID;
		}
		else {
			return 0;
		}
	}

	void Find_AvailableAttributes(aiMesh* data, Attribute_BitMask& Available_Attributes) {
		if (data->HasNormals()) {
			Available_Attributes.Normal = true;
		}
		else {
			Available_Attributes.Normal = false;
		}
		//Maximum Available UV Set Number is 4
		Available_Attributes.TextCoords = 0;
		for (unsigned int i = 0; i < 4; i++) {
			if (data->HasTextureCoords(i)) {
				Available_Attributes.TextCoords++;
			}
		}
	}

	//Return EdgeIndex
	unsigned int CreateEdge(Static_Mesh_Data* Mesh, unsigned int V1, unsigned int V2) {
		Vertex& V1o = Mesh->Vertexes[V1];
		Vertex& V2o = Mesh->Vertexes[V2];
		for (unsigned int EdgeIndex = 0; EdgeIndex < V1o.CommonEdges.size(); EdgeIndex++) {
			Edge& V1E = Mesh->Edges[V1o.CommonEdges[EdgeIndex]];
			if (V1E.VertexIDs[0] == V2 || V1E.VertexIDs[1] == V2) {
				return V1o.CommonEdges[EdgeIndex];
			}
		}


		unsigned int EdgeIndex = Mesh->Edges.size();
		Mesh->Edges.push_back(Edge());
		Edge& NewEdge = Mesh->Edges[EdgeIndex];
		//Always the vertex that has lower ID is VertexIDs[0]
		if (V1 < V2) {
			NewEdge.VertexIDs[0] = V1;
			NewEdge.VertexIDs[1] = V2;
		}
		else {
			NewEdge.VertexIDs[0] = V2;
			NewEdge.VertexIDs[1] = V1;
		}
		NewEdge.Length = glm::length(V1o.Position - V2o.Position);
		V1o.CommonEdges.push_back(EdgeIndex);
		V2o.CommonEdges.push_back(EdgeIndex);
		return EdgeIndex;
	}
	void LinkEdge_and_Triangle(Static_Mesh_Data* Mesh, unsigned int EdgeID, unsigned int TriangleID) {
		Triangle& Triangle =  Mesh->Triangles[TriangleID];
		//Select which TriangleID to modify
		unsigned int i = 0;
		if (Mesh->Edges[EdgeID].TriangleIDs[0] != UINT32_MAX) {
			i = 1;
		}
		Mesh->Edges[EdgeID].TriangleIDs[i] = TriangleID;

		//Find unused edge array index in triangles array
		unsigned int UnusedEdgeIndex = 0;
		if (Triangle.EdgeIDs[0] != UINT32_MAX) {
			UnusedEdgeIndex++;
			if (Triangle.EdgeIDs[1] != UINT32_MAX) {
				UnusedEdgeIndex++;
				if (Triangle.EdgeIDs[2] != UINT32_MAX) {
					LOG_CRASHING("Wait WTF?");
				}
			}
		}
		
		Triangle.EdgeIDs[UnusedEdgeIndex] = EdgeID;
	}
	void Load_MeshData(const aiMesh* data, Attribute_BitMask& Attribute_Info, Static_Model* Model) {
		Static_Mesh_Data* MESH = new Static_Mesh_Data;
		Model->MESHes.push_back(MESH);

		//You should choose the Vertex Attribute Layout, then set the data according to it. So I should create a Attribute Layout Settings window here.
		//And give the user which attributes the imported asset has.
		MESH->Material_Index = data->mMaterialIndex;
		if (data->mNumVertices == 4563) {
			LOG_CRASHING("This is it!");
		}
		MESH->VERTEX_NUMBER = data->mNumVertices;

		//Mesh Attribute Loading
		{
			GFX_API::VertexAttribute PositionAttribute;
			//Create Position Attribute!
			PositionAttribute.AttributeName = "Positions";
			PositionAttribute.Index = 0;
			PositionAttribute.Stride = 0;
			PositionAttribute.Start_Offset = 0;
			PositionAttribute.DATATYPE = GFX_API::DATA_TYPE::VAR_VEC3;
			MESH->DataLayout.Attributes.push_back(PositionAttribute);

			//Use this index to set Attribute index for each attribute
			unsigned int NextAttribute_Index = 1;

			/*
			//Load UV Sets!
			for (unsigned int i = 0; i < Attribute_Info.TextCoords; i++) {
				GFX_API::VertexAttribute TextCoordAttribute;
				TextCoordAttribute.AttributeName.append("UV Set ");
				TextCoordAttribute.AttributeName.append(std::to_string(i).c_str());
				TextCoordAttribute.Index = NextAttribute_Index + i;
				TextCoordAttribute.Stride = 0;
				TextCoordAttribute.Start_Offset = 0;
				switch (data->mNumUVComponents[i]) {
				case 2:
					TextCoordAttribute.DATATYPE = GFX_API::DATA_TYPE::VAR_VEC2;
					break;
				default:
					TextCoordAttribute.DATATYPE = GFX_API::DATA_TYPE::VAR_VEC2;
					LOG_WARNING("One of the meshes has unsupported number of channels in its Texture Coordinates!");
				}
				MESH->DataLayout.Attributes.push_back(TextCoordAttribute);
				NextAttribute_Index++;
			}*/

			if (Attribute_Info.Normal) {
				GFX_API::VertexAttribute NormalAttribute;
				NormalAttribute.AttributeName = "Normals";
				NormalAttribute.Index = NextAttribute_Index;
				NextAttribute_Index++;
				NormalAttribute.Stride = 0;
				NormalAttribute.Start_Offset = 0;
				NormalAttribute.DATATYPE = GFX_API::DATA_TYPE::VAR_VEC3;
				MESH->DataLayout.Attributes.push_back(NormalAttribute);
			}


			//Final Checks on Attribute Layout
			if (!MESH->DataLayout.VerifyAttributeLayout()) {
				LOG_ERROR("Attribute Layout isn't verified, there is a problem somewhere!");
				return;
			}
			MESH->DataLayout.Calculate_SizeperVertex();
		}

		MESH->VERTEXDATA_SIZE = MESH->DataLayout.size_pervertex * MESH->VERTEX_NUMBER;
		MESH->VERTEX_DATA = new unsigned char[MESH->VERTEXDATA_SIZE];
		if (!MESH->VERTEX_DATA) {
			LOG_CRASHING("Allocator failed to create a buffer for mesh's data!");
			return;
		}


		//Filling MESH->DATA!
		{
			//First, fill the position buffer
			memcpy(MESH->VERTEX_DATA, data->mVertices, MESH->VERTEX_NUMBER *  sizeof(vec3));
			
			unsigned char* NEXTDATA_PTR = (unsigned char*)MESH->VERTEX_DATA + (MESH->VERTEX_NUMBER * sizeof(vec3));
			unsigned char NEXTATTRIB_INDEX = 1;
			GFX_API::VertexAttributeLayout& LAYOUT = MESH->DataLayout;

			/*
			if (Attribute_Info.TextCoords) {
				for (unsigned int i = 0; i < Attribute_Info.TextCoords; i++) {
					vec2* TEXTCOORD_PTR = (vec2*)NEXTDATA_PTR;
					GFX_API::VertexAttribute& TEXTCOORD_ATTRIB = LAYOUT.Attributes[NEXTATTRIB_INDEX];
					unsigned int data_size = MESH->VERTEX_NUMBER * 8;
					for (unsigned int j = 0; j < data_size / 8; j++) {
						TEXTCOORD_PTR[j] = vec2(data->mTextureCoords[i][j].x, data->mTextureCoords[i][j].y);
					}
					NEXTDATA_PTR += data_size;
					NEXTATTRIB_INDEX++;
				}
			}*/
			if (Attribute_Info.Normal) {
				GFX_API::VertexAttribute& NORMAL_ATTRIB = LAYOUT.Attributes[NEXTATTRIB_INDEX];
				unsigned int data_size = MESH->VERTEX_NUMBER * GFX_API::Get_UNIFORMTYPEs_SIZEinbytes(NORMAL_ATTRIB.DATATYPE);
				memcpy(NEXTDATA_PTR, data->mNormals, data_size);
				NEXTDATA_PTR += data_size;
				NEXTATTRIB_INDEX++;
			}
		}

		//Filling MESH->INDEXDATA!
		if (data->HasFaces()) {
			MESH->INDEX_DATA = new unsigned int[data->mNumFaces * 3];
			for (unsigned int i = 0; i < data->mNumFaces; i++) {
				MESH->INDEX_DATA[i * 3] = data->mFaces[i].mIndices[0];
				MESH->INDEX_DATA[i * 3 + 1] = data->mFaces[i].mIndices[1];
				MESH->INDEX_DATA[i * 3 + 2] = data->mFaces[i].mIndices[2];
			}
			MESH->INDICES_NUMBER = data->mNumFaces * 3;
		}

		//This needs a huge fix because data structure has changed a little bit
		/*
		MESH->Vertexes = new Vertex[MESH->VERTEX_NUMBER];
		MESH->Triangles = new Triangle[MESH->INDICES_NUMBER / 3];
		for (unsigned int VertexIndex = 0; VertexIndex < MESH->VERTEX_NUMBER; VertexIndex++) {
			Vertex& VData = MESH->Vertexes[VertexIndex]; 
			VData.VertexID = VertexIndex;
			VData.Position.x = data->mVertices[VertexIndex].x;
			VData.Position.y = data->mVertices[VertexIndex].y;
			VData.Position.z = data->mVertices[VertexIndex].z;
		}
		for (unsigned int TriangleIndex = 0; TriangleIndex < MESH->INDICES_NUMBER / 3; TriangleIndex++) {
			MESH->Triangles[TriangleIndex].VertexIDs[0] = MESH->INDEX_DATA[TriangleIndex * 3];
			MESH->Triangles[TriangleIndex].VertexIDs[1] = MESH->INDEX_DATA[TriangleIndex * 3 + 1];
			MESH->Triangles[TriangleIndex].VertexIDs[2] = MESH->INDEX_DATA[TriangleIndex * 3 + 2];
			unsigned int Edge1Index = CreateEdge(MESH, MESH->INDEX_DATA[TriangleIndex * 3], MESH->INDEX_DATA[TriangleIndex * 3 + 1]);
			unsigned int Edge2Index = CreateEdge(MESH, MESH->INDEX_DATA[TriangleIndex * 3], MESH->INDEX_DATA[TriangleIndex * 3 + 2]);
			unsigned int Edge3Index = CreateEdge(MESH, MESH->INDEX_DATA[TriangleIndex * 3 + 1], MESH->INDEX_DATA[TriangleIndex * 3 + 2]);
			for (unsigned int VertexTriangleIndex = 0; VertexTriangleIndex < 3; VertexTriangleIndex++) {
				Vertex& Vertex = MESH->Vertexes[MESH->INDEX_DATA[TriangleIndex * 3 + VertexTriangleIndex]];
				Vertex.CommonTriangles.push_back(TriangleIndex);
			}
			LinkEdge_and_Triangle(MESH, Edge1Index, TriangleIndex);
			LinkEdge_and_Triangle(MESH, Edge2Index, TriangleIndex);
			LinkEdge_and_Triangle(MESH, Edge3Index, TriangleIndex);
			if (MESH->Triangles[TriangleIndex].EdgeIDs[0] == -1 ||
				MESH->Triangles[TriangleIndex].EdgeIDs[1] == -1 ||
				MESH->Triangles[TriangleIndex].EdgeIDs[2] == -1) {
				LOG_CRASHING("Wait WTF?");
			}
		}*/
	}


	unsigned int Load_MaterialData(const aiMaterial* data, const char* directory, unsigned int* object_worldid) {
		SURFACEMAT_PROPERTIES properties;
		if (data->GetTextureCount(aiTextureType_DIFFUSE)) {
			aiString PATH;
			data->GetTexture(aiTextureType_DIFFUSE, 0, &PATH);
			string dir = (string)directory + '/';
			string texture_path = dir + (string)PATH.C_Str();
			properties.DIFFUSETEXTURE_ID = Texture_Loader::Import_Texture(texture_path.c_str())->ID;
		}
		if (data->GetTextureCount(aiTextureType_HEIGHT)) {
			aiString PATH;
			data->GetTexture(aiTextureType_HEIGHT, 0, &PATH);
			string dir = (string)directory + '/';
			string texture_path = dir + (string)PATH.C_Str();
			properties.NORMALSTEXTURE_ID = Texture_Loader::Import_Texture(texture_path.c_str())->ID;
		}
		if (data->GetTextureCount(aiTextureType_SPECULAR)) {
			aiString PATH;
			data->GetTexture(aiTextureType_SPECULAR, 0, &PATH);
			string dir = (string)directory + '/';
			string texture_path = dir + (string)PATH.C_Str();
			properties.SPECULARTEXTURE_ID = Texture_Loader::Import_Texture(texture_path.c_str())->ID;
		}
		if (data->GetTextureCount(aiTextureType_OPACITY)) {
			aiString PATH;
			data->GetTexture(aiTextureType_OPACITY, 0, &PATH);
			string dir = (string)directory + '/';
			string texture_path = dir + (string)PATH.C_Str();
			properties.OPACITYTEXTURE_ID = Texture_Loader::Import_Texture(texture_path.c_str())->ID;
		}

		return RenderDataManager::Create_SurfaceMaterialInstance(properties, object_worldid);
	}
}