#include "EditorFileSystem_Core.h"
#include "TuranAPI/Logger_Core.h"

//Data Formats created by Flatbuffers!
#include <flatbuffers/flatbuffers.h>
#include "Editor/FileSystem/ResourceTypes/ResourceTYPEs.h"
#include "ResourceTypes/Model_Resource.h"
#include "ResourceTypes/Scene_Resource.h"

namespace TuranEditor {
	Editor_FileSystem* Editor_FileSystem::SELF = nullptr;

	Editor_FileSystem::Editor_FileSystem() : Asset_IDSet(100) {
		LOG_STATUS("Starting the Editor FileSystem! But this doesn't make anything!");
		SELF = this;
	}

	void Editor_FileSystem::Add_anAsset_toAssetList(Resource_Identifier* RESOURCE) {
		unsigned int ID = Create_Resource_ID();
		if (RESOURCE == nullptr) {
			LOG_ERROR("Editor FileSystem couldn't add the asset to File List, because Resource is nullptr!");
			return;
		}
		
		RESOURCE->ID = ID;
		Asset_List.push_back(RESOURCE);
	}
	void Editor_FileSystem::Delete_anAsset_fromAssetList(unsigned int ID) {
		for (unsigned int i = 0; i < Asset_List.size(); i++) {
			Resource_Identifier* ASSET = Asset_List[i];
			if (ID == ASSET->ID) {
				Delete_Resource_ID(ASSET->ID);
				Asset_List.erase(Asset_List.begin() + i);
				return;
			}
		}
		LOG_ERROR("Editor_FileSystem tried to delete an Asset, but it is not in FileList! There is a lack in somewhere!");
	}

	bool Editor_FileSystem::Does_ResourceExist(unsigned int Resource_ID, RESOURCETYPEs TYPE) const {
		for (Resource_Identifier* resource : Asset_List) {
			if (Resource_ID == resource->ID && TYPE == resource->TYPE) {
				return true;
			}
		}
		LOG_WARNING("Does_ResourceExist() failed to find a resource!\n");
		return false;
	}


	Resource_Identifier* Editor_FileSystem::Find_ResourceIdentifier_byID(unsigned int ID) {
		for (Resource_Identifier* RESOURCE : Asset_List) {
			if (RESOURCE->ID == ID) {
				return RESOURCE;
			}
		}
		LOG_ERROR("Find_ResourceIdentifier_byID has failed to find the resource!");
		return nullptr;
	}


	const vector<Resource_Identifier*>& Editor_FileSystem::Get_AssetList() const {
		return Asset_List;
	}
	vector<Resource_Identifier*> Editor_FileSystem::Get_SpecificAssetType(RESOURCETYPEs TYPE) {
		vector<Resource_Identifier*> All_Assets;
		for (unsigned int i = 0; i < Asset_List.size(); i++) {
			Resource_Identifier* RESOURCE = Asset_List[i];
			if (RESOURCE->TYPE == TYPE) {
				All_Assets.push_back(RESOURCE);
			}
		}
		return All_Assets;
	}




	//Asset ID's should be bigger than 0, 0 is invalid value for Asset ID's.
	//But Bitset starts from index 0, so we should add 1.
	unsigned int Editor_FileSystem::Create_Resource_ID() {
		size_t availableID = Asset_IDSet.GetIndex_FirstFalse() + 1;
		Asset_IDSet.SetBit_True(availableID - 1);
		return availableID;
	}
	void Editor_FileSystem::Read_ID(unsigned int Asset_ID) {
		Asset_IDSet.SetBit_True(Asset_ID - 1);
	}
	void Editor_FileSystem::Delete_Resource_ID(unsigned int Asset_ID) {
		Asset_IDSet.SetBit_False(Asset_ID - 1);
	}

}