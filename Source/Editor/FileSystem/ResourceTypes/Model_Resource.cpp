#include "Model_Resource.h"
#include "Resource_Identifier.h"
#include "GFX/GFX_Core.h"

namespace TuranEditor {
	Static_Mesh_Data::Static_Mesh_Data(){

	}
	Static_Mesh_Data::~Static_Mesh_Data() {
		delete VERTEX_DATA;
		delete INDEX_DATA;
	}
	bool Static_Mesh_Data::Verify_Mesh_Data() {
		if (VERTEX_NUMBER == 0){
			LOG_ERROR("Vertex number is zero or below!");
			return false;
		}
		else if (!DataLayout.VerifyAttributeLayout()) {
			LOG_ERROR("Attribute Layout isn't verified, so Mesh_Data isn't verified!");
			return false;
		}
		return true;
	}



	//Static Model Data class function definitions

	Static_Model::Static_Model() {

	}
	Static_Model::~Static_Model() {
		MESHes.clear();
	}

	vector<unsigned int> Static_Model::Upload_toGPU() {
		vector<unsigned int> MeshBuffer_IDs;
		for (unsigned int i = 0; i < MESHes.size(); i++) {
			const Static_Mesh_Data* MESH = MESHes[i];
			unsigned int MESHBUFFER_ID = GFXContentManager->Upload_MeshBuffer(MESH->DataLayout, MESH->VERTEX_DATA, MESH->VERTEXDATA_SIZE, MESH->VERTEX_NUMBER, MESH->INDEX_DATA, MESH->INDICES_NUMBER);
			MeshBuffer_IDs.push_back(MESHBUFFER_ID);
		}
		return MeshBuffer_IDs;
	}
}