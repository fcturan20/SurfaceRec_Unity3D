#include "MC.h"
#include "GFX/GFX_Core.h"
#include "Editor/FileSystem/EditorFileSystem_Core.h"
#include "TuranAPI/FileSystem_Core.h"
#include "Editor/FileSystem/ResourceTypes/ResourceTYPEs.h"



#include "Editor/TUBITAK/MarchingCubes_LookUpTable.h"
MC_ComputePass::MC_ComputePass() {
	unsigned int lookuptable_ub_id = GFXContentManager->Create_GlobalBuffer("LookupTable", MarchingCubes_LookUpTable, 256 * 16, GFX_API::BUFFER_VISIBILITY::CPUEXISTENCE_GPUREADONLY);
	GFX_API::ComputeShader_Resource* cs_res = new GFX_API::ComputeShader_Resource;
	cs_res->LANGUAGE = GFX_API::SHADER_LANGUAGEs::GLSL;
	cs_res->SOURCE_CODE = *TAPIFILESYSTEM::Read_TextFile((string(SOURCE_DIR) + "/Content/MC.comp").c_str());

	unsigned int SDFsamples_id = GFXContentManager->Create_GlobalBuffer("SDFSamples", nullptr, 80 * 80 * 80, GFX_API::BUFFER_VISIBILITY::CPUREADWRITE_GPUREADWRITE);
	//Set Global Buffers
	{
		GFX_API::GlobalBuffer_Access lookup_access;
		lookup_access.ACCESS_TYPE = GFX_API::OPERATION_TYPE::READ_ONLY;
		lookup_access.BUFFER_ID = lookuptable_ub_id;
		cs_res->GLOBALBUFFERs.push_back(lookup_access);

		GFX_API::GlobalBuffer_Access SDFSamples;
		lookup_access.ACCESS_TYPE = GFX_API::OPERATION_TYPE::READ_ONLY;
		lookup_access.BUFFER_ID = SDFsamples_id;
		cs_res->GLOBALBUFFERs.push_back(SDFSamples);
	}
	//Set Uniforms
	{
		GFX_API::Material_Uniform SamplingD_uni;
		SamplingD_uni.VARIABLE_NAME = "SamplingD";
		SamplingD_uni.VARIABLE_TYPE = GFX_API::DATA_TYPE::VAR_FLOAT32;
		cs_res->UNIFORMs.push_back(SamplingD_uni);

		GFX_API::Material_Uniform BOUNDINGMIN_uni;
		BOUNDINGMIN_uni.VARIABLE_NAME = "BOUNDINGMIN";
		BOUNDINGMIN_uni.VARIABLE_TYPE = GFX_API::DATA_TYPE::VAR_VEC3;
		cs_res->UNIFORMs.push_back(BOUNDINGMIN_uni);

		GFX_API::Material_Uniform BOUNDINGMAX_uni;
		BOUNDINGMAX_uni.VARIABLE_NAME = "BOUNDINGMAX";
		BOUNDINGMAX_uni.VARIABLE_TYPE = GFX_API::DATA_TYPE::VAR_VEC3;
		cs_res->UNIFORMs.push_back(BOUNDINGMAX_uni);
	}
	TuranEditor::Resource_Identifier* res = new TuranEditor::Resource_Identifier;
	res->TYPE = TuranEditor::RESOURCETYPEs::GFXAPI_COMPUTESHADER;
	TuranEditor::EDITOR_FILESYSTEM->Add_anAsset_toAssetList(res);
	
	std::string comp_status;
	GFXContentManager->Compile_ComputeShader(cs_res, res->ID, &comp_status);

}
void MC_ComputePass::Execute() {
}
void MC_ComputePass::RunMC(std::vector<float> SDFSamples, dvec3 BOUNDINGMIN, dvec3 BOUNDINGMAX, float samplingD) {

}