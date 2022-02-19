#include "MC.h"
#include "GFX/GFX_Core.h"
#include "Editor/FileSystem/EditorFileSystem_Core.h"
#include "TuranAPI/FileSystem_Core.h"
#include "Editor/FileSystem/ResourceTypes/ResourceTYPEs.h"
#include "Editor/TUBITAK/MarchingCubes_LookUpTable.h"
#include <glm/glm.hpp>
static unsigned int CS_TYPE_ID = 0, CS_INST_ID = 0, LOOKUPTABLE_UB_ID = 0, SDFSAMPLES_ID = 0, MCRESULTS_ID = 0;
static vec3* results;
static bool should_execute = false;

//Uniforms
static float samplingD = 0.0;
static vec3 boundingMIN(-FLT_MAX), boundingMAX(FLT_MAX);
static unsigned int SDFinputs_res = 0;

//Buffers
static float* SDFinputs = new float[80 * 80 * 80];

MC_ComputePass::MC_ComputePass() {
	unsigned int lookuptable_ub_id = GFXContentManager->Create_GlobalBuffer("LookupTable", MarchingCubes_LookUpTable, 256 * 16, GFX_API::BUFFER_VISIBILITY::CPUEXISTENCE_GPUREADONLY);
	GFX_API::ComputeShader_Resource* cs_type = new GFX_API::ComputeShader_Resource;
	cs_type->LANGUAGE = GFX_API::SHADER_LANGUAGEs::GLSL;
	cs_type->SOURCE_CODE = *TAPIFILESYSTEM::Read_TextFile((string(SOURCE_DIR) + "/Content/MC.comp").c_str());

	SDFSAMPLES_ID = GFXContentManager->Create_GlobalBuffer("SDFSamples", nullptr, 80 * 80 * 80 * 4, GFX_API::BUFFER_VISIBILITY::CPUREADWRITE_GPUREADWRITE);
	MCRESULTS_ID = GFXContentManager->Create_GlobalBuffer("MCResults", nullptr, 79 * 79 * 79 * 15 * 12, GFX_API::BUFFER_VISIBILITY::CPUREADWRITE_GPUREADWRITE);
	//Set Global Buffers
	{
		GFX_API::GlobalBuffer_Access lookup_access;
		lookup_access.ACCESS_TYPE = GFX_API::OPERATION_TYPE::READ_ONLY;
		lookup_access.BUFFER_ID = lookuptable_ub_id;
		cs_type->GLOBALBUFFERs.push_back(lookup_access);

		GFX_API::GlobalBuffer_Access SDFSamples;
		SDFSamples.ACCESS_TYPE = GFX_API::OPERATION_TYPE::READ_ONLY;
		SDFSamples.BUFFER_ID = SDFSAMPLES_ID;
		cs_type->GLOBALBUFFERs.push_back(SDFSamples);

		GFX_API::GlobalBuffer_Access MCResults;
		MCResults.ACCESS_TYPE = GFX_API::OPERATION_TYPE::READ_AND_WRITE;
		MCResults.BUFFER_ID = MCRESULTS_ID;
		cs_type->GLOBALBUFFERs.push_back(MCResults);
	}
	//Set Uniforms
	{
		GFX_API::Material_Uniform SamplingD_uni;
		SamplingD_uni.VARIABLE_NAME = "SamplingD";
		SamplingD_uni.VARIABLE_TYPE = GFX_API::DATA_TYPE::VAR_FLOAT32;
		cs_type->UNIFORMs.push_back(SamplingD_uni);

		GFX_API::Material_Uniform BOUNDINGMIN_uni;
		BOUNDINGMIN_uni.VARIABLE_NAME = "BOUNDINGMIN";
		BOUNDINGMIN_uni.VARIABLE_TYPE = GFX_API::DATA_TYPE::VAR_VEC3;
		cs_type->UNIFORMs.push_back(BOUNDINGMIN_uni);

		GFX_API::Material_Uniform BOUNDINGMAX_uni;
		BOUNDINGMAX_uni.VARIABLE_NAME = "BOUNDINGMAX";
		BOUNDINGMAX_uni.VARIABLE_TYPE = GFX_API::DATA_TYPE::VAR_VEC3;
		cs_type->UNIFORMs.push_back(BOUNDINGMAX_uni);
	} 
	{
		TuranEditor::Resource_Identifier* cs_res = new TuranEditor::Resource_Identifier;
		cs_res->TYPE = TuranEditor::RESOURCETYPEs::GFXAPI_COMPUTESHADER;
		TuranEditor::EDITOR_FILESYSTEM->Add_anAsset_toAssetList(cs_res);

		std::string comp_status;
		GFXContentManager->Compile_ComputeShader(cs_type, cs_res->ID, &comp_status);
		CS_TYPE_ID = cs_res->ID;
	}


	GFX_API::ComputeShader_Instance* cs_ins = new GFX_API::ComputeShader_Instance;
	cs_ins->ComputeShader = CS_TYPE_ID;
	cs_ins->UNIFORM_LIST = cs_type->UNIFORMs;
	for (unsigned int i = 0; i < cs_ins->UNIFORM_LIST.size(); i++) {
		GFX_API::Material_Uniform& uni = cs_ins->UNIFORM_LIST[i];
		if (uni.VARIABLE_NAME == "SamplingD" && uni.VARIABLE_TYPE == GFX_API::DATA_TYPE::VAR_FLOAT32) { uni.DATA = &samplingD;}
		if (uni.VARIABLE_NAME == "BOUNDINGMIN" && uni.VARIABLE_TYPE == GFX_API::DATA_TYPE::VAR_VEC3) { uni.DATA = &boundingMIN; }
		if (uni.VARIABLE_NAME == "BOUNDINGMAX" && uni.VARIABLE_TYPE == GFX_API::DATA_TYPE::VAR_VEC3) { uni.DATA = &boundingMAX; }
	}
	//Resource Compilation
	{
		TuranEditor::Resource_Identifier* RESOURCE = new TuranEditor::Resource_Identifier;
		RESOURCE->TYPE = TuranEditor::RESOURCETYPEs::GFXAPI_COMPUTESHADERINST;
		RESOURCE->DATA = cs_ins;
		TuranEditor::EDITOR_FILESYSTEM->Add_anAsset_toAssetList(RESOURCE);
		string compilation_status;		//I don't care, because it will be compiled anyway!
		CS_INST_ID = RESOURCE->ID;
		ComputeShaders.push_back(cs_ins);
	}
}
void MC_ComputePass::Execute() {
	if (!should_execute) { return; }
	//SDFSamples Upload
	GFXContentManager->Upload_GlobalBuffer(SDFSAMPLES_ID);
	GFXRENDERER->Compute_Dispatch(ComputeShaders[0], vec3(SDFinputs_res / 8, SDFinputs_res / 8, 1));
	const void* dataloc = GFXContentManager->StartReading_GlobalBuffer(MCRESULTS_ID, GFX_API::OPERATION_TYPE::READ_ONLY);
	memcpy(results, dataloc, 79 * 79 * 79 * 15 * 12);
	GFXContentManager->FinishReading_GlobalBuffer(MCRESULTS_ID);
	should_execute = false;
}
void MC_ComputePass::RunMC(std::vector<float> SDFSamples, dvec3 BOUNDINGMIN, dvec3 BOUNDINGMAX, float SAMPLINGD, unsigned int SDFRes, vec3* result) {
	if (should_execute) { LOG_CRASHING("You shouldn't call RunMC more than once in a frame!"); return; }
	memcpy(SDFinputs, SDFSamples.data(), SDFSamples.size() * sizeof(float));
	boundingMIN = BOUNDINGMIN;
	boundingMAX = BOUNDINGMAX;
	samplingD = SAMPLINGD;
	SDFinputs_res = SDFRes;
	results = result;
	should_execute = true;
}