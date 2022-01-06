#include "PointCloudImports.h"
#include "Editor/TUBITAK/DataLoader.h"
#include "Editor/RenderContext/Editor_DataManager.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Editor/FileSystem/EditorFileSystem_Core.h"
#include "Editor/FileSystem/ResourceTypes/ResourceTYPEs.h"
#include "Editor/FileSystem/ResourceImporters/Model_Loader.h"

static std::string PATH = "C:/Users/furka/Desktop/Meshes/HumanBase.off"
//PointCloudPATH = "C:/Users/furka/Desktop/Meshes/PCDs and Blensor/Plane00000.pcd"
, PCViewerError = "", IMPORTNAME = "";
static PCViewer* viewer = nullptr;


void GetVertexes_FromPolygonalModel(const char* PATH, vector<vec3>& VertexPositions, vector<vec3>& VertexNormals) {
	//Check if model is available
	Assimp::Importer import;
	const aiScene* Scene = nullptr;
	{
		Scene = import.ReadFile(PATH, aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_FindInvalidData);

		//Check if scene reading errors!
		if (!Scene || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Scene->mRootNode) {
			LOG_ERROR("Failed on Loading Mesh with Assimp; " + string(import.GetErrorString()));
			return;
		}

		if (Scene->mNumMeshes == 0) {
			LOG_ERROR("Failed because there is no mesh in loaded scene!");
			return;
		}
	}

	//Importing process
	VertexPositions.clear();
	VertexNormals.clear();
	unsigned int VertexCount = 0;
	for (unsigned int i = 0; i < Scene->mNumMeshes; i++) {
		VertexCount += Scene->mMeshes[i]->mNumVertices;
	}
	VertexPositions.resize(VertexCount);
	unsigned int StartIndex = 0;
	for (unsigned int i = 0; i < Scene->mNumMeshes; i++) {
		//First, fill the position buffer
		memcpy(&VertexPositions[StartIndex], Scene->mMeshes[i]->mVertices, Scene->mMeshes[i]->mNumVertices * sizeof(vec3));
		StartIndex += Scene->mMeshes[i]->mNumVertices;
	}
	bool NormalCompatible = true;
	for (unsigned int i = 0; i < Scene->mNumMeshes; i++) {
		if (!Scene->mMeshes[i]->HasNormals()) {
			NormalCompatible = false;
			break;
		}
	}
	//Load normals
	if (NormalCompatible) {
		StartIndex = 0;
		VertexNormals.resize(VertexCount);
		for (unsigned int i = 0; i < Scene->mNumMeshes; i++) {
			//First, fill the position buffer
			memcpy(&VertexNormals[StartIndex], Scene->mMeshes[i]->mNormals, Scene->mMeshes[i]->mNumVertices * sizeof(vec3));
			StartIndex += Scene->mMeshes[i]->mNumVertices;
		}
	}
}

void GetNonIndexedVertexes_FromPolygonalModel(const char* PATH, vector<vec3>& VertexPositions, vector<vec3>& VertexNormals) {
	//Check if model is available
	Assimp::Importer import;
	const aiScene* Scene = nullptr;
	{
		Scene = import.ReadFile(PATH, aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_FindInvalidData);

		//Check if scene reading errors!
		if (!Scene || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Scene->mRootNode) {
			LOG_ERROR("Failed on Loading Mesh with Assimp; " + string(import.GetErrorString()));
			return;
		}

		if (Scene->mNumMeshes == 0) {
			LOG_ERROR("Failed because there is no mesh in loaded scene!");
			return;
		}
	}

	//Importing process
	VertexPositions.clear();
	VertexNormals.clear();
	unsigned int VertexCount = 0;
	for (unsigned int i = 0; i < Scene->mNumMeshes; i++) {
		VertexCount += Scene->mMeshes[i]->mNumFaces * 3;
	}
	VertexPositions.resize(VertexCount); VertexNormals.resize(VertexCount);

	unsigned int LastIndex = 0;
	for (unsigned int MeshIndex = 0; MeshIndex < Scene->mNumMeshes; MeshIndex++) {
		for (unsigned int FaceID = 0; FaceID < Scene->mMeshes[MeshIndex]->mNumFaces; FaceID++) {
			auto v0 = Scene->mMeshes[MeshIndex]->mVertices[Scene->mMeshes[MeshIndex]->mFaces[FaceID].mIndices[0]];
			auto v1 = Scene->mMeshes[MeshIndex]->mVertices[Scene->mMeshes[MeshIndex]->mFaces[FaceID].mIndices[1]];
			auto v2 = Scene->mMeshes[MeshIndex]->mVertices[Scene->mMeshes[MeshIndex]->mFaces[FaceID].mIndices[2]];
			auto n0 = Scene->mMeshes[MeshIndex]->mNormals[Scene->mMeshes[MeshIndex]->mFaces[FaceID].mIndices[0]];
			auto n1 = Scene->mMeshes[MeshIndex]->mNormals[Scene->mMeshes[MeshIndex]->mFaces[FaceID].mIndices[1]];
			auto n2 = Scene->mMeshes[MeshIndex]->mNormals[Scene->mMeshes[MeshIndex]->mFaces[FaceID].mIndices[2]];
			VertexPositions[LastIndex] = vec3(v0.x, v0.y, v0.z);
			VertexPositions[LastIndex + 1] = vec3(v1.x, v1.y, v1.z);
			VertexPositions[LastIndex + 2] = vec3(v2.x, v2.y, v2.z);
			VertexNormals[LastIndex] = vec3(n0.x, n0.y, n0.z);
			VertexNormals[LastIndex + 1] = vec3(n1.x, n1.y, n1.z);
			VertexNormals[LastIndex + 2] = vec3(n2.x, n2.y, n2.z);
			LastIndex += 3;
		}
	}
}

void PointCloudImports::ImportPolygonalModel(PCViewer* Viewer) {
	IMGUI->Input_Text("PATH", &PATH);
	IMGUI->Input_Text("Import Name", &IMPORTNAME);
	if (IMGUI->Begin_TabBar()) {
		if (IMGUI->Begin_TabItem("Import Seperately as PC")) {
			if (IMGUI->Button("Import")) {
				PointCloud* LoadedPC = TuranEditor::DataLoader::LoadMesh_asPointCloud(PATH.c_str());
				if (LoadedPC) {
					PCViewer::PointCloud_DD* PC_DD = new PCViewer::PointCloud_DD;
					PC_DD->PATH = PATH;
					PC_DD->NAME = IMPORTNAME;
					PC_DD->isVisible = true;
					PC_DD->PC.DifferentRepresentations = LoadedPC->DifferentRepresentations;
					PC_DD->PC.PointCount = LoadedPC->PointCount;
					PC_DD->PC.PointNormals = LoadedPC->PointNormals;
					PC_DD->PC.PointPositions = LoadedPC->PointPositions;
					delete LoadedPC;

					PC_DD->PCRenderer = TuranEditor::RenderDataManager::Create_PointRenderer(PC_DD->PC.PointCount);
					if (PC_DD->PCRenderer == nullptr) {
						LOG_CRASHING("Creating the point buffer on GPU has failed!");
					}
					else {
						for (unsigned int i = 0; i < PC_DD->PC.PointCount; i++) {
							PC_DD->PCRenderer->GetPointPosition_byIndex(i) = PC_DD->PC.PointPositions[i];
							PC_DD->PCRenderer->GetPointCOLORRGBA_byIndex(i) = vec4(1.0, 0.0, 0.0, 1.0);
							PC_DD->CenterOfData += PC_DD->PC.PointPositions[i];
						}
					}
					PC_DD->CenterOfData = PC_DD->CenterOfData / dvec3(PC_DD->PC.PointCount);
					vec3 BOUNDINGMIN(FLT_MAX), BOUNDINGMAX(FLT_MIN);
					for (unsigned int i = 0; i < PC_DD->PC.PointCount; i++) {
						double dist = length(PC_DD->CenterOfData - dvec3(PC_DD->PC.PointPositions[i]));
						if (dist > PC_DD->BoundingSphereRadius) { PC_DD->BoundingSphereRadius = dist; }

						if (PC_DD->PC.PointPositions[i].x < BOUNDINGMIN.x) { BOUNDINGMIN.x = PC_DD->PC.PointPositions[i].x; }
						if (PC_DD->PC.PointPositions[i].y < BOUNDINGMIN.y) { BOUNDINGMIN.y = PC_DD->PC.PointPositions[i].y; }
						if (PC_DD->PC.PointPositions[i].z < BOUNDINGMIN.z) { BOUNDINGMIN.z = PC_DD->PC.PointPositions[i].z; }

						if (PC_DD->PC.PointPositions[i].x > BOUNDINGMAX.x) { BOUNDINGMAX.x = PC_DD->PC.PointPositions[i].x; }
						if (PC_DD->PC.PointPositions[i].y > BOUNDINGMAX.y) { BOUNDINGMAX.y = PC_DD->PC.PointPositions[i].y; }
						if (PC_DD->PC.PointPositions[i].z > BOUNDINGMAX.z) { BOUNDINGMAX.z = PC_DD->PC.PointPositions[i].z; }
					}
					PC_DD->BOUNDINGMAX = BOUNDINGMAX;	PC_DD->BOUNDINGMIN = BOUNDINGMIN;

					Viewer->DisplayableDatas.push_back(PC_DD);
				}
				else {
					PCViewerError = "Import failed!";
				}

			}
			IMGUI->End_TabItem();
		}
		if (IMGUI->Begin_TabItem("Import and Merge as PC")) {
			static unsigned int selectedlistitemindex = 0, selectedddindex = 0; static bool isselected = false;
			if (Viewer->SelectListOneLine_FromDisplayableDatas(PCViewer::DisplayableData::DataType::POINTCLOUD, selectedlistitemindex, selectedddindex, "Point Cloud to Merge Into")) {
				isselected = true;
			}
			if (isselected) {
				if (IMGUI->Button("Import and Merge")) {
					vector<vec3> VertexPositions, VertexNormals;
					GetVertexes_FromPolygonalModel(PATH.c_str(), VertexPositions, VertexNormals);


					PCViewer::PointCloud_DD* ImportedPC = new PCViewer::PointCloud_DD;
					PCViewer::PointCloud_DD* MergedPC = static_cast<PCViewer::PointCloud_DD*>(Viewer->DisplayableDatas[selectedddindex]);

					ImportedPC->NAME = MergedPC->NAME + " + " + IMPORTNAME;
					ImportedPC->PATH = "Merged";
					ImportedPC->isVisible = true;

					ImportedPC->PC.PointCount = MergedPC->PC.PointCount + VertexPositions.size();
					ImportedPC->PC.PointPositions = new vec3[ImportedPC->PC.PointCount]{ vec3(0) };
					for (unsigned int i = 0; i < MergedPC->PC.PointCount; i++) {
						ImportedPC->PC.PointPositions[i] = vec3(MergedPC->PC.PointPositions[i].x, MergedPC->PC.PointPositions[i].y, MergedPC->PC.PointPositions[i].z);
					}
					for (unsigned int i = 0; i < VertexPositions.size(); i++) {
						ImportedPC->PC.PointPositions[i + MergedPC->PC.PointCount] = VertexPositions[i];
					}


					//Upload the data, calculate the center and the bounding sphere radius
					vec3 BOUNDINGMIN(FLT_MAX), BOUNDINGMAX(FLT_MIN);
					for (unsigned int i = 0; i < ImportedPC->PC.PointCount; i++) {
						ImportedPC->CenterOfData += ImportedPC->PC.PointPositions[i];
						if (ImportedPC->PC.PointPositions[i].x < BOUNDINGMIN.x) { BOUNDINGMIN.x = ImportedPC->PC.PointPositions[i].x; }
						if (ImportedPC->PC.PointPositions[i].y < BOUNDINGMIN.y) { BOUNDINGMIN.y = ImportedPC->PC.PointPositions[i].y; }
						if (ImportedPC->PC.PointPositions[i].z < BOUNDINGMIN.z) { BOUNDINGMIN.z = ImportedPC->PC.PointPositions[i].z; }

						if (ImportedPC->PC.PointPositions[i].x > BOUNDINGMAX.x) { BOUNDINGMAX.x = ImportedPC->PC.PointPositions[i].x; }
						if (ImportedPC->PC.PointPositions[i].y > BOUNDINGMAX.y) { BOUNDINGMAX.y = ImportedPC->PC.PointPositions[i].y; }
						if (ImportedPC->PC.PointPositions[i].z > BOUNDINGMAX.z) { BOUNDINGMAX.z = ImportedPC->PC.PointPositions[i].z; }
					}
					ImportedPC->BOUNDINGMAX = BOUNDINGMAX; ImportedPC->BOUNDINGMIN = BOUNDINGMIN;
					ImportedPC->CenterOfData = ImportedPC->CenterOfData / dvec3(ImportedPC->PC.PointCount);
					for (unsigned int i = 0; i < ImportedPC->PC.PointCount; i++) {
						double dist = length(ImportedPC->CenterOfData - dvec3(ImportedPC->PC.PointPositions[i]));
						if (dist > ImportedPC->BoundingSphereRadius) { ImportedPC->BoundingSphereRadius = dist; }
					}

					ImportedPC->PCRenderer = TuranEditor::RenderDataManager::Create_PointRenderer(ImportedPC->PC.PointCount);
					if (ImportedPC->PCRenderer == nullptr) {
						LOG_CRASHING("Creating the point buffer on GPU has failed!");
					}
					for (unsigned int i = 0; i < ImportedPC->PC.PointCount; i++) {
						ImportedPC->PCRenderer->GetPointPosition_byIndex(i) = ImportedPC->PC.PointPositions[i];
						ImportedPC->PCRenderer->GetPointCOLORRGBA_byIndex(i) = vec4(1.0f, 0.0f, 0.0f, 1.0f);
					}

					Viewer->DisplayableDatas.push_back(ImportedPC);
				}
			}


			IMGUI->End_TabItem();
		}
		if (IMGUI->Begin_TabItem("Import as Triangle Model")) {
			if (IMGUI->Button("Import")) {
				vector<vec3> NORMALs, POSITIONs;
				GetNonIndexedVertexes_FromPolygonalModel(PATH.c_str(), POSITIONs, NORMALs);
				
				if (POSITIONs.size() == 0) {
					LOG_CRASHING("This file isn't supported!"); IMGUI->End_TabItem(); IMGUI->End_TabBar(); return;
				}
				unsigned int MESHBUFFERID = UINT32_MAX;
				{
					vector<vec3> uploaddata(POSITIONs.size() * 2, vec3(0));
					memcpy(uploaddata.data(), POSITIONs.data(), POSITIONs.size() * 12);
					memcpy(uploaddata.data() + POSITIONs.size(), NORMALs.data(), POSITIONs.size() * 12);
					MESHBUFFERID = GFXContentManager->Upload_MeshBuffer(TuranEditor::RenderDataManager::PositionNormal_VertexAttrib, uploaddata.data(), uploaddata.size() * 12, POSITIONs.size(), nullptr, 0);
				}
				
				PCViewer::TriangleModel* MODEL = new PCViewer::TriangleModel;
				MODEL->GPUMESH_IDs.push_back(MESHBUFFERID);
				MODEL->DisplayedVertexBuffers.push_back(true);
				MODEL->isVisible = true;
				MODEL->NAME = IMPORTNAME;
				MODEL->PATH = PATH;
				MODEL->NonIndexed_VertexBuffers.push_back(POSITIONs);
				PCViewer::TriangleModel::TriangleModelNormals originalnormal;
				originalnormal.NORMALNAME = "Original";
				originalnormal.NonIndexed_VertexNormals.push_back(NORMALs);
				MODEL->VertexNormalTYPEs.push_back(originalnormal);

				//Get unique vertexes to find center of the model
				GetVertexes_FromPolygonalModel(PATH.c_str(), POSITIONs, NORMALs);
				unsigned int TotalVertexCount = 0;
				vec3 BOUNDINGMIN(FLT_MAX), BOUNDINGMAX(FLT_MIN);
				for (unsigned int UniqueVertexID = 0; UniqueVertexID < POSITIONs.size(); UniqueVertexID++) {
					MODEL->CenterOfData += POSITIONs[UniqueVertexID];
					if (POSITIONs[UniqueVertexID].x < BOUNDINGMIN.x) { BOUNDINGMIN.x = POSITIONs[UniqueVertexID].x; }
					if (POSITIONs[UniqueVertexID].y < BOUNDINGMIN.y) { BOUNDINGMIN.y = POSITIONs[UniqueVertexID].y; }
					if (POSITIONs[UniqueVertexID].z < BOUNDINGMIN.z) { BOUNDINGMIN.z = POSITIONs[UniqueVertexID].z; }

					if (POSITIONs[UniqueVertexID].x > BOUNDINGMAX.x) { BOUNDINGMAX.x = POSITIONs[UniqueVertexID].x; }
					if (POSITIONs[UniqueVertexID].y > BOUNDINGMAX.y) { BOUNDINGMAX.y = POSITIONs[UniqueVertexID].y; }
					if (POSITIONs[UniqueVertexID].z > BOUNDINGMAX.z) { BOUNDINGMAX.z = POSITIONs[UniqueVertexID].z; }
				}
				MODEL->BOUNDINGMIN = BOUNDINGMIN; MODEL->BOUNDINGMAX = BOUNDINGMAX;
				MODEL->CenterOfData = MODEL->CenterOfData / dvec3(POSITIONs.size());
				for (unsigned int UniqueVertexID = 0; UniqueVertexID < POSITIONs.size(); UniqueVertexID++) {
					double dist = length(MODEL->CenterOfData - dvec3(POSITIONs[UniqueVertexID]));
					if (dist > MODEL->BoundingSphereRadius) { MODEL->BoundingSphereRadius = dist; }
				}

				Viewer->DisplayableDatas.push_back(MODEL);
			}
			IMGUI->End_TabItem();
		}

		IMGUI->End_TabBar();
	}
	IMGUI->Text(PCViewerError.c_str());
}

#include "Editor/FileSystem/DataFormats/JobDatas_generated.h"
void Import_PointCloud(const Resource* RESOURCE_typeless) {
	auto RESOURCE = RESOURCE_typeless->TYPE_as_PointCloud();

	if (!RESOURCE) { LOG_CRASHING("Type isn't Point Cloud, Type definition is wrong!"); IMGUI->End_TabItem(); IMGUI->End_TabBar(); return; }

	PCViewer::PointCloud_DD* pc_dd = new PCViewer::PointCloud_DD;
	pc_dd->NAME = IMPORTNAME.c_str();
	pc_dd->PATH = PATH;
	pc_dd->PC.PointCount = RESOURCE->Positions()->size();
	pc_dd->PC.PointPositions = new vec3[pc_dd->PC.PointCount];
	pc_dd->isVisible = true;
	for (unsigned int i = 0; i < pc_dd->PC.PointCount; i++) {
		pc_dd->PC.PointPositions[i] = vec3(RESOURCE->Positions()->Get(i)->x(), RESOURCE->Positions()->Get(i)->y(), RESOURCE->Positions()->Get(i)->z());
	}
	pc_dd->PC.PointNormals.resize(RESOURCE->NormalLists()->size());
	for (unsigned int NormalListIndex = 0; NormalListIndex < pc_dd->PC.PointNormals.size(); NormalListIndex++) {
		PC_PointNormals& normallist = pc_dd->PC.PointNormals[NormalListIndex];
		auto fb_normallist = RESOURCE->NormalLists()->Get(NormalListIndex);
		normallist.NAME = fb_normallist->NAME()->c_str();
		normallist.Normals = new vec3[pc_dd->PC.PointCount];
		for (unsigned int i = 0; i < pc_dd->PC.PointCount; i++) {
			normallist.Normals[i] = vec3(fb_normallist->Normals()->Get(i)->x(), fb_normallist->Normals()->Get(i)->y(), fb_normallist->Normals()->Get(i)->z());
		}
	}
	pc_dd->PCRenderer = TuranEditor::RenderDataManager::Create_PointRenderer(pc_dd->PC.PointCount);
	for (unsigned int i = 0; i < pc_dd->PC.PointCount; i++) {
		pc_dd->PCRenderer->GetPointPosition_byIndex(i) = pc_dd->PC.PointPositions[i];
		pc_dd->PCRenderer->GetPointCOLORRGBA_byIndex(i) = vec4(1.0, 0.0, 0.0, 1.0);
	}
	pc_dd->BOUNDINGMAX = dvec3(-DBL_MAX);
	pc_dd->BOUNDINGMIN = dvec3(DBL_MAX);
	//Calculate the center and the bounding sphere radius
	for (unsigned int i = 0; i < pc_dd->PC.PointCount; i++) {
		pc_dd->CenterOfData += pc_dd->PC.PointPositions[i];

		if(pc_dd->PC.PointPositions[i].x < pc_dd->BOUNDINGMIN.x){pc_dd->BOUNDINGMIN.x = pc_dd->PC.PointPositions[i].x;}
		if(pc_dd->PC.PointPositions[i].y < pc_dd->BOUNDINGMIN.y){pc_dd->BOUNDINGMIN.y = pc_dd->PC.PointPositions[i].y;}
		if(pc_dd->PC.PointPositions[i].z < pc_dd->BOUNDINGMIN.z){pc_dd->BOUNDINGMIN.z = pc_dd->PC.PointPositions[i].z;}

		if(pc_dd->PC.PointPositions[i].x > pc_dd->BOUNDINGMAX.x){pc_dd->BOUNDINGMAX.x = pc_dd->PC.PointPositions[i].x;}
		if(pc_dd->PC.PointPositions[i].y > pc_dd->BOUNDINGMAX.y){pc_dd->BOUNDINGMAX.y = pc_dd->PC.PointPositions[i].y;}
		if(pc_dd->PC.PointPositions[i].z > pc_dd->BOUNDINGMAX.z){pc_dd->BOUNDINGMAX.z = pc_dd->PC.PointPositions[i].z;}
	}
	pc_dd->CenterOfData = pc_dd->CenterOfData / dvec3(pc_dd->PC.PointCount);
	for (unsigned int i = 0; i < pc_dd->PC.PointCount; i++) {
		double dist = length(pc_dd->CenterOfData - dvec3(pc_dd->PC.PointPositions[i]));
		if (dist > pc_dd->BoundingSphereRadius) { pc_dd->BoundingSphereRadius = dist; }
	}

	pc_dd->PCRenderer = TuranEditor::RenderDataManager::Create_PointRenderer(pc_dd->PC.PointCount);
	if (pc_dd->PCRenderer == nullptr) {
		LOG_CRASHING("Creating the point buffer on GPU has failed!");
	}
	for (unsigned int i = 0; i < pc_dd->PC.PointCount; i++) {
		pc_dd->PCRenderer->GetPointPosition_byIndex(i) = pc_dd->PC.PointPositions[i];
		pc_dd->PCRenderer->GetPointCOLORRGBA_byIndex(i) = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}

	viewer->DisplayableDatas.push_back(pc_dd);
}
void Import_TriangleModel(const Resource* RESOURCE_typeless) {
	auto RESOURCE = RESOURCE_typeless->TYPE_as_DGSMODEL();
	if (!RESOURCE) { LOG_CRASHING("Type isn't Model, Type definition is wrong!"); IMGUI->End_TabItem(); IMGUI->End_TabBar(); return; }

	PCViewer::TriangleModel* tri_dd = new PCViewer::TriangleModel;
	tri_dd->BOUNDINGMAX = dvec3(RESOURCE->BOUNDINGMAX()->x(), RESOURCE->BOUNDINGMAX()->y(), RESOURCE->BOUNDINGMAX()->z());
	tri_dd->BOUNDINGMIN = dvec3(RESOURCE->BOUNDINGMIN()->x(), RESOURCE->BOUNDINGMIN()->y(), RESOURCE->BOUNDINGMIN()->z());
	tri_dd->CenterOfData = dvec3(RESOURCE->CENTER()->x(), RESOURCE->CENTER()->y(), RESOURCE->CENTER()->z());
	tri_dd->BoundingSphereRadius = RESOURCE->BOUNDINGSPHERERADIUS();
	tri_dd->INFO = RESOURCE->INFO()->c_str();
	tri_dd->isVisible = true;
	tri_dd->NAME = IMPORTNAME;
	tri_dd->PATH = PATH;
	tri_dd->NonIndexed_VertexBuffers.resize(RESOURCE->MESHes()->size());
	tri_dd->VertexNormalTYPEs.resize(RESOURCE->MESHes()->Get(0)->NormalLists()->size());
	for (unsigned int MeshIndex = 0; MeshIndex < tri_dd->NonIndexed_VertexBuffers.size(); MeshIndex++) {
		auto FB_MESH = RESOURCE->MESHes()->Get(MeshIndex);
		tri_dd->NonIndexed_VertexBuffers[MeshIndex].resize(FB_MESH->Positions()->size());
		for (unsigned int VertexID = 0; VertexID < FB_MESH->Positions()->size(); VertexID++) {
			const Vec3f* fb_vertex = FB_MESH->Positions()->Get(VertexID);
			tri_dd->NonIndexed_VertexBuffers[MeshIndex][VertexID] = dvec3(fb_vertex->x(), fb_vertex->y(), fb_vertex->z());
		}
	}
	for (unsigned int NormalListIndex = 0; NormalListIndex < tri_dd->VertexNormalTYPEs.size(); NormalListIndex++) {
		tri_dd->VertexNormalTYPEs[NormalListIndex].NonIndexed_VertexNormals.resize(tri_dd->NonIndexed_VertexBuffers.size());
		for (unsigned int MeshIndex = 0; MeshIndex < tri_dd->NonIndexed_VertexBuffers.size(); MeshIndex++) {
			vector<vec3>& normallist = tri_dd->VertexNormalTYPEs[NormalListIndex].NonIndexed_VertexNormals[MeshIndex];
			normallist.resize(tri_dd->NonIndexed_VertexBuffers[MeshIndex].size());
			auto fb_normallist = RESOURCE->MESHes()->Get(MeshIndex)->NormalLists()->Get(NormalListIndex)->Normals();
			for (unsigned int VertexIndex = 0; VertexIndex < tri_dd->NonIndexed_VertexBuffers[MeshIndex].size(); VertexIndex++) {
				normallist[VertexIndex] = vec3(fb_normallist->Get(VertexIndex)->x(), fb_normallist->Get(VertexIndex)->y(), fb_normallist->Get(VertexIndex)->z());
			}
		}
		tri_dd->VertexNormalTYPEs[NormalListIndex].NORMALNAME = RESOURCE->MESHes()->Get(0)->NormalLists()->Get(NormalListIndex)->NAME()->c_str();
	}

	tri_dd->DisplayedVertexBuffers.resize(tri_dd->NonIndexed_VertexBuffers.size(), true);
	tri_dd->GPUMESH_IDs.resize(tri_dd->NonIndexed_VertexBuffers.size(), UINT32_MAX);
	for (unsigned int MeshIndex = 0; MeshIndex < tri_dd->NonIndexed_VertexBuffers.size(); MeshIndex++) {
		vector<vec3> VertexBuffer(tri_dd->NonIndexed_VertexBuffers[MeshIndex].size() * 2, vec3(UINT32_MAX));
		memcpy(VertexBuffer.data(), tri_dd->NonIndexed_VertexBuffers[MeshIndex].data(), tri_dd->NonIndexed_VertexBuffers[MeshIndex].size() * 12);
		memcpy(VertexBuffer.data() + tri_dd->NonIndexed_VertexBuffers[MeshIndex].size(), tri_dd->VertexNormalTYPEs[0].NonIndexed_VertexNormals[MeshIndex].data(), tri_dd->NonIndexed_VertexBuffers[MeshIndex].size() * 12);
		unsigned int MeshBufferID = GFXContentManager->Upload_MeshBuffer(TuranEditor::RenderDataManager::PositionNormal_VertexAttrib, VertexBuffer.data(), VertexBuffer.size() * 12,
			VertexBuffer.size() / 2, nullptr, 0);
		tri_dd->GPUMESH_IDs[MeshIndex] = MeshBufferID;
	}

	viewer->DisplayableDatas.push_back(tri_dd);
}
void Export_PointCloud(unsigned int selectedddindex) {
	PCViewer::PointCloud_DD* pc_dd = static_cast<PCViewer::PointCloud_DD*>(viewer->DisplayableDatas[selectedddindex]);
	if (!pc_dd->PC.PointCount) { LOG_CRASHING("Point cloud is empty, check it!"); IMGUI->End_TabItem(); IMGUI->End_TabBar(); return; }

	flatbuffers::FlatBufferBuilder fbb(1024);

	vector<Vec3f> FB_POSITIONs(pc_dd->PC.PointCount);
	for (unsigned int PointIndex = 0; PointIndex < pc_dd->PC.PointCount; PointIndex++) {
		FB_POSITIONs[PointIndex] = Vec3f(pc_dd->PC.PointPositions[PointIndex].x, pc_dd->PC.PointPositions[PointIndex].y, pc_dd->PC.PointPositions[PointIndex].z);
	}

	vector<flatbuffers::Offset<NormalsStruct>> FB_NORMALLIST(pc_dd->PC.PointNormals.size());
	vector<vector<Vec3f>> FB_NORMALs(pc_dd->PC.PointNormals.size());
	for (unsigned int NormalListIndex = 0; NormalListIndex < pc_dd->PC.PointNormals.size(); NormalListIndex++) {
		PC_PointNormals& normallist = pc_dd->PC.PointNormals[NormalListIndex];
		vector<Vec3f>& fb_normal = FB_NORMALs[NormalListIndex];
		fb_normal.resize(pc_dd->PC.PointCount);
		for (unsigned int i = 0; i < fb_normal.size(); i++) {
			fb_normal[i] = Vec3f(normallist.Normals[i].x, normallist.Normals[i].y, normallist.Normals[i].z);
		}
		FB_NORMALLIST[NormalListIndex] = CreateNormalsStructDirect(fbb, normallist.NAME.c_str(), &FB_NORMALs[NormalListIndex]);
	}

	auto PC_Var = CreatePC_StructDirect(fbb, &FB_POSITIONs, 0, &FB_NORMALLIST);
	auto Resource_var = CreateResource(fbb, Resource_Type_PointCloud, PC_Var.Union());
	fbb.Finish(Resource_var);
	void* data = fbb.GetBufferPointer();
	unsigned int datasize = fbb.GetSize();

	TuranAPI::FileSystem::Write_BinaryFile(PATH.c_str(), data, datasize);
}
void Export_TriangleModel(unsigned int selectedddindex) {
	PCViewer::TriangleModel* tri_dd = static_cast<PCViewer::TriangleModel*>(viewer->DisplayableDatas[selectedddindex]);
	if (tri_dd->NonIndexed_VertexBuffers.size() == 0) { LOG_CRASHING("Triangle model hasn't vertex buffer!"); return; }

	flatbuffers::FlatBufferBuilder fbb(1024);
	vector<flatbuffers::Offset<DGS_Mesh>> FB_Meshes(tri_dd->NonIndexed_VertexBuffers.size());
	//This is to prevent array of positions and normals from dying
	unsigned int normals_array_count = 0;
	for (unsigned int NormallistIndex = 0; NormallistIndex < tri_dd->VertexNormalTYPEs.size(); NormallistIndex++) {
		normals_array_count += tri_dd->VertexNormalTYPEs[NormallistIndex].NonIndexed_VertexNormals.size();
	}
	vector<vector<Vec3f>> POSITIONs_and_NORMALs_BACKUP(tri_dd->NonIndexed_VertexBuffers.size() + normals_array_count);
	vector<vector<flatbuffers::Offset<NormalsStruct>>> FB_NORMALLISTs_BACKUP(tri_dd->NonIndexed_VertexBuffers.size());
	vector<string> MeshNames(tri_dd->NonIndexed_VertexBuffers.size());

	unsigned int current_backup_index = 0;
	for (unsigned int MeshIndex = 0; MeshIndex < FB_Meshes.size(); MeshIndex++) {
		vector<Vec3f>* position_ptr = &POSITIONs_and_NORMALs_BACKUP[current_backup_index++];
		position_ptr->resize(tri_dd->NonIndexed_VertexBuffers[MeshIndex].size());
		for (unsigned int VertexID = 0; VertexID < position_ptr->size(); VertexID++) {
			(*position_ptr)[VertexID] = Vec3f(tri_dd->NonIndexed_VertexBuffers[MeshIndex][VertexID].x,
				tri_dd->NonIndexed_VertexBuffers[MeshIndex][VertexID].y,
				tri_dd->NonIndexed_VertexBuffers[MeshIndex][VertexID].z);
		}

		for (unsigned int NormalListIndex = 0; NormalListIndex < tri_dd->VertexNormalTYPEs.size(); NormalListIndex++) {
			if (tri_dd->VertexNormalTYPEs[NormalListIndex].NonIndexed_VertexNormals.size() <= MeshIndex) { continue; }

			const vector<vec3>& normallist = tri_dd->VertexNormalTYPEs[NormalListIndex].NonIndexed_VertexNormals[MeshIndex];

			vector<Vec3f>* fb_normalptr = &POSITIONs_and_NORMALs_BACKUP[current_backup_index];
			fb_normalptr->resize(tri_dd->NonIndexed_VertexBuffers[MeshIndex].size());

			for (unsigned int vertexindex = 0; vertexindex < fb_normalptr->size(); vertexindex++) {
				(*fb_normalptr)[vertexindex] = Vec3f(tri_dd->VertexNormalTYPEs[NormalListIndex].NonIndexed_VertexNormals[MeshIndex][vertexindex].x,
					tri_dd->VertexNormalTYPEs[NormalListIndex].NonIndexed_VertexNormals[MeshIndex][vertexindex].y,
					tri_dd->VertexNormalTYPEs[NormalListIndex].NonIndexed_VertexNormals[MeshIndex][vertexindex].z);
			}

			FB_NORMALLISTs_BACKUP[MeshIndex].push_back(CreateNormalsStructDirect(fbb, tri_dd->VertexNormalTYPEs[NormalListIndex].NORMALNAME.c_str(),
				fb_normalptr));
			current_backup_index++;
		}

		MeshNames[MeshIndex] = to_string(MeshIndex).c_str();
		FB_Meshes[MeshIndex] = CreateDGS_MeshDirect(fbb, position_ptr, &FB_NORMALLISTs_BACKUP[MeshIndex], MeshNames[MeshIndex].c_str());
	}

	Vec3f boundingmin = Vec3f(tri_dd->BOUNDINGMIN.x, tri_dd->BOUNDINGMIN.y, tri_dd->BOUNDINGMIN.z), 
		boundingmax = Vec3f(tri_dd->BOUNDINGMAX.x, tri_dd->BOUNDINGMAX.y, tri_dd->BOUNDINGMAX.z), 
		center = Vec3f(tri_dd->CenterOfData.x, tri_dd->CenterOfData.y, tri_dd->CenterOfData.z);

	auto TRI_Var = CreateDGS_ModelDirect(fbb, &FB_Meshes, tri_dd->INFO.c_str(), &boundingmin, &boundingmax, &center, tri_dd->BoundingSphereRadius);
	auto Resource_var = CreateResource(fbb, Resource_Type_DGSMODEL, TRI_Var.Union());
	fbb.Finish(Resource_var);
	void* data = fbb.GetBufferPointer();
	unsigned int datasize = fbb.GetSize();

	TuranAPI::FileSystem::Write_BinaryFile(PATH.c_str(), data, datasize);
}
void PointCloudImports::ImportExport_DGSFile(PCViewer* Viewer) {
	viewer = Viewer;
	if (!IMGUI->Begin_TabBar()) { return; }

	if (IMGUI->Begin_TabItem("Import")) {
		IMGUI->Input_Text("PATH", &PATH);
		IMGUI->Input_Text("NAME", &IMPORTNAME);


		if (IMGUI->Button("Import")) {
			unsigned int datasize = 0;
			void* data = TuranAPI::FileSystem::Read_BinaryFile(PATH.c_str(), datasize);
			if (!data) { LOG_CRASHING("File isn't found or not binary!"); IMGUI->End_TabItem(); IMGUI->End_TabBar(); return; }

			auto RESOURCE_typeless = GetResource(data);
			if (RESOURCE_typeless == nullptr) { LOG_CRASHING("Loading failed! File isn't a valid resource!"); IMGUI->End_TabItem(); IMGUI->End_TabBar(); return; }
			if (RESOURCE_typeless->TYPE_as_PointCloud()) { Import_PointCloud(RESOURCE_typeless); }
			else if (RESOURCE_typeless->TYPE_as_DGSMODEL()) { Import_TriangleModel(RESOURCE_typeless); }
			else {LOG_CRASHING("This type of DGS file doesn't support importing!");}
		}

		IMGUI->End_TabItem();
	}
	if (IMGUI->Begin_TabItem("Export")) {
		static unsigned int selectedlistitemindex = 0, selectedddindex = 0;
		Viewer->SelectListOneLine_FromDisplayableDatas(PCViewer::DisplayableData::ALL, selectedlistitemindex, selectedddindex, "PC");
		if (selectedddindex == UINT32_MAX) { return; }

		IMGUI->Input_Text("Output PATH", &PATH);
		if (IMGUI->Button("Export")) {
			switch (Viewer->DisplayableDatas[selectedddindex]->TYPE) {
			case PCViewer::DisplayableData::POINTCLOUD:
				Export_PointCloud(selectedddindex); break;
			case PCViewer::DisplayableData::TRIANGLEMODEL:
				Export_TriangleModel(selectedddindex); break;
			default:
				IMGUI->Text("Data type doesn't support exporting!");
			}
		}
		IMGUI->End_TabItem();
	}
	IMGUI->End_TabBar();
}
/*
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/filters/filter.h>*/
void PointCloudImports::ImportPCDFile(PCViewer* Viewer) {
	/*
	IMGUI->Input_Text("Point Cloud PATH", &PATH);
	IMGUI->Input_Text("Import Name", &IMPORTNAME);
	if (IMGUI->Begin_TabBar()) {
		if (IMGUI->Begin_TabItem("Import Seperately")) {
			if(IMGUI->Button("Import")) {
				pcl::PointCloud<pcl::PointXYZ> PCLCloud;
				if (pcl::io::loadPCDFile<pcl::PointXYZ>(PATH, PCLCloud) != -1) {
					PCLCloud.is_dense = false;
					pcl::PointCloud<pcl::PointXYZ> noNaNPCLCloud;
					std::vector<pcl::index_t> ind;
					pcl::removeNaNFromPointCloud(PCLCloud, noNaNPCLCloud, ind);

					PCViewer::PointCloud_DD* ImportedPC = new PCViewer::PointCloud_DD;

					ImportedPC->NAME = IMPORTNAME;
					ImportedPC->PATH = PATH;
					ImportedPC->isVisible = true;
					ImportedPC->TYPE = PCViewer::PointCloud_DD::POINTCLOUD;

					ImportedPC->PC.PointCount = noNaNPCLCloud.size();
					ImportedPC->PC.PointPositions = new vec3[ImportedPC->PC.PointCount]{ vec3(0) };
					for (unsigned int i = 0; i < ImportedPC->PC.PointCount; i++) {
						ImportedPC->PC.PointPositions[i] = vec3(noNaNPCLCloud.points[i].x, noNaNPCLCloud.points[i].y, noNaNPCLCloud.points[i].z);
						ImportedPC->PC.PointPositions[i] *= 20.0;
					}


					//Calculate the center and the bounding sphere radius
					for (unsigned int i = 0; i < ImportedPC->PC.PointCount; i++) {
						ImportedPC->CenterOfData += ImportedPC->PC.PointPositions[i];
					}
					ImportedPC->CenterOfData = ImportedPC->CenterOfData / dvec3(ImportedPC->PC.PointCount);
					for (unsigned int i = 0; i < ImportedPC->PC.PointCount; i++) {
						double dist = length(ImportedPC->CenterOfData - dvec3(ImportedPC->PC.PointPositions[i]));
						if (dist > ImportedPC->BoundingSphereRadius) { ImportedPC->BoundingSphereRadius = dist; }
					}

					ImportedPC->PCRenderer = TuranEditor::RenderDataManager::Create_PointRenderer(ImportedPC->PC.PointCount);
					if (ImportedPC->PCRenderer == nullptr) {
						LOG_CRASHING("Creating the point buffer on GPU has failed!");
					}
					for (unsigned int i = 0; i < ImportedPC->PC.PointCount; i++) {
						ImportedPC->PCRenderer->GetPointPosition_byIndex(i) = ImportedPC->PC.PointPositions[i];
						ImportedPC->PCRenderer->GetPointCOLORRGBA_byIndex(i) = vec4(1.0f, 0.0f, 0.0f, 1.0f);
					}

					Viewer->DisplayableDatas.push_back(ImportedPC);
				}
				else {
					PCViewerError = "PCD file isn't found!";
				}
			}
			IMGUI->End_TabItem();
		}
		if (IMGUI->Begin_TabItem("Import and Merge")) {
			static unsigned int selectedlistitemindex = 0, selectedddindex = 0; static bool isselected = false;
			if (Viewer->SelectListOneLine_FromDisplayableDatas(PCViewer::DisplayableData::DataType::POINTCLOUD, selectedlistitemindex, selectedddindex, "Point Cloud to Merge Into")) {
				isselected = true;
			}
			if (isselected) {
				if (IMGUI->Button("Merge")) {

					pcl::PointCloud<pcl::PointXYZ> PCLCloud;
					if (pcl::io::loadPCDFile<pcl::PointXYZ>(PATH, PCLCloud) != -1) {
						PCLCloud.is_dense = false;
						pcl::PointCloud<pcl::PointXYZ> noNaNPCLCloud;
						std::vector<pcl::index_t> ind;
						pcl::removeNaNFromPointCloud(PCLCloud, noNaNPCLCloud, ind);

						PCViewer::PointCloud_DD* ImportedPC = new PCViewer::PointCloud_DD;
						PCViewer::PointCloud_DD* MergedPC = static_cast<PCViewer::PointCloud_DD*>(Viewer->DisplayableDatas[selectedddindex]);

						ImportedPC->NAME = MergedPC->NAME + " + " + IMPORTNAME;
						ImportedPC->PATH = "Merged";
						ImportedPC->isVisible = true;
						ImportedPC->TYPE = PCViewer::PointCloud_DD::POINTCLOUD;

						ImportedPC->PC.PointCount = MergedPC->PC.PointCount + noNaNPCLCloud.size();
						ImportedPC->PC.PointPositions = new vec3[ImportedPC->PC.PointCount]{ vec3(0) };
						for (unsigned int i = 0; i < MergedPC->PC.PointCount; i++) {
							ImportedPC->PC.PointPositions[i] = vec3(MergedPC->PC.PointPositions[i].x, MergedPC->PC.PointPositions[i].y, MergedPC->PC.PointPositions[i].z);
							ImportedPC->PC.PointPositions[i] *= 20.0;
						}
						for (unsigned int i = 0; i < noNaNPCLCloud.size(); i++) {
							ImportedPC->PC.PointPositions[i + MergedPC->PC.PointCount] = vec3(noNaNPCLCloud[i].x, noNaNPCLCloud[i].y, noNaNPCLCloud[i].z);
							ImportedPC->PC.PointPositions[i + MergedPC->PC.PointCount] *= 20.0;
						}


						//Calculate the center and the bounding sphere radius
						for (unsigned int i = 0; i < ImportedPC->PC.PointCount; i++) {
							ImportedPC->CenterOfData += ImportedPC->PC.PointPositions[i];
						}
						ImportedPC->CenterOfData = ImportedPC->CenterOfData / dvec3(ImportedPC->PC.PointCount);
						for (unsigned int i = 0; i < ImportedPC->PC.PointCount; i++) {
							double dist = length(ImportedPC->CenterOfData - dvec3(ImportedPC->PC.PointPositions[i]));
							if (dist > ImportedPC->BoundingSphereRadius) { ImportedPC->BoundingSphereRadius = dist; }
						}

						ImportedPC->PCRenderer = TuranEditor::RenderDataManager::Create_PointRenderer(ImportedPC->PC.PointCount);
						if (ImportedPC->PCRenderer == nullptr) {
							LOG_CRASHING("Creating the point buffer on GPU has failed!");
						}
						for (unsigned int i = 0; i < ImportedPC->PC.PointCount; i++) {
							ImportedPC->PCRenderer->GetPointPosition_byIndex(i) = ImportedPC->PC.PointPositions[i];
							ImportedPC->PCRenderer->GetPointCOLORRGBA_byIndex(i) = vec4(1.0f, 0.0f, 0.0f, 1.0f);
						}

						Viewer->DisplayableDatas.push_back(ImportedPC);
					}

					isselected = false;
				}
			}


			IMGUI->End_TabItem();
		}

		IMGUI->End_TabBar();
	}
	IMGUI->Text(PCViewerError.c_str());
	*/
}