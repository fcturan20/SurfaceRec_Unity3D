#pragma once
#include "Editor/Editor_Includes.h"
#include "ResourceTypes/Resource_Identifier.h"
#include "TuranAPI/Bitset.h"

/*
1) This namespace is defined in Engine
2) But extended in Editor to handle Editor specific files too!
*/
namespace TuranEditor {
	/*
	1) This class is used to load the project's editor datas;
	2) You can specify how to create and load new data types here!
	3) If project's File_List.bin isn't found, gives error to either specify the location or create a new project!
	*/
	class Editor_FileSystem {
		vector<Resource_Identifier*> Asset_List;
		Bitset Asset_IDSet;


		//Asset ID's should be bigger than 0, 0 is invalid value for Asset ID's.
		//But Bitset starts from index 0, so we should add 1.
		unsigned int Create_Resource_ID(); 
		void Read_ID(unsigned int Asset_ID);
		void Delete_Resource_ID(unsigned int Asset_ID);

	public:
		Editor_FileSystem();
		static Editor_FileSystem* SELF;

		//FILE LIST OPERATIONs
		void Add_anAsset_toAssetList(Resource_Identifier* RESOURCE);
		void Delete_anAsset_fromAssetList(unsigned int ID);
		bool Does_ResourceExist(unsigned int Resource_ID, RESOURCETYPEs TYPE) const;
		const vector<Resource_Identifier*>& Get_AssetList() const;
		vector<Resource_Identifier*> Get_SpecificAssetType(RESOURCETYPEs TYPE);


		Resource_Identifier* Find_ResourceIdentifier_byID(unsigned int ID);
	};
#define EDITOR_FILESYSTEM Editor_FileSystem::SELF
}