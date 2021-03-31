#include "DataLoader.h"
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
#include "Editor/FileSystem/EditorFileSystem_Core.h"

namespace TuranEditor {
	PointCloud* DataLoader::LoadMesh_asPointCloud(const char* PATH) {
		//Check if model is available
		Assimp::Importer import;
		const aiScene* Scene = nullptr;
		{
			Scene = import.ReadFile(PATH, aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_FindInvalidData);

			//Check if scene reading errors!
			if (!Scene || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Scene->mRootNode) {
				std::cout << (("Failed on Loading Mesh with Assimp; " + string(import.GetErrorString())).c_str()) << std::endl;
				return nullptr;
			}

			if (Scene->mNumMeshes == 0) {
				std::cout << "Failed because there is no mesh in loaded scene!" << std::endl;
				return nullptr;
			}
		}

		//Importing process
		PointCloud* LoadedCloud = new PointCloud;
		for (unsigned int i = 0; i < Scene->mNumMeshes; i++) {
			LoadedCloud->PointCount += Scene->mMeshes[i]->mNumVertices;
		}
		LoadedCloud->PointPositions = new vec3[LoadedCloud->PointCount];
		unsigned int StartIndex = 0;
		for (unsigned int i = 0; i < Scene->mNumMeshes; i++) {
			//First, fill the position buffer
			memcpy(&LoadedCloud->PointPositions[StartIndex], Scene->mMeshes[i]->mVertices, Scene->mMeshes[i]->mNumVertices * sizeof(vec3));
			StartIndex += Scene->mMeshes[i]->mNumVertices;
		}


		TuranEditor::Resource_Identifier* RESOURCE = new Resource_Identifier;
		RESOURCE->PATH = PATH;
		RESOURCE->DATA = LoadedCloud;
		RESOURCE->TYPE = RESOURCETYPEs::EDITOR_POINTCLOUD;
		TuranEditor::EDITOR_FILESYSTEM->Add_anAsset_toAssetList(RESOURCE);
		return LoadedCloud;
	}
}