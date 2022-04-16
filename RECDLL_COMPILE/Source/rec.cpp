#include "TuranAPI/ThreadedJobCore.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "rec.h"
#include <mutex>
#include <chrono>
#include <ctime>

//Multi-threading
static std::atomic_uint processing_jobindex = 0, finished_jobindex = 0;
static unsigned int sdf_jobcount = 24, mc_jobcount = 24, sdf_elementcount_perjob, mc_elementcount_perjob, sdf_lastjob_elementcount, mc_lastjob_elementcount;
static vector<float> SDFValues; static std::vector<unsigned int> SDF_nearestpointindexes;
static vector<vector<vec3>> MC_Results;
static unsigned int SDFRes = 10;
static float SamplingD = 0.0;
static dvec3 BOUNDINGMIN, BOUNDINGMAX, SAMPLE_DIST;
static std::mutex* SDF_lockers = nullptr;

uint32_t PC_pointcount = 0;
vec3* PC_positions = nullptr, *PC_normals = nullptr;


inline vec3 FindSampleLoc_fromSampleIndex(unsigned int SamplePointIndex){
		unsigned int SampleIndex_Z = SamplePointIndex / (SDFRes * SDFRes);
		unsigned int SampleIndex_Y = (SamplePointIndex - (SampleIndex_Z * SDFRes * SDFRes)) / SDFRes;
		unsigned int SampleIndex_X = SamplePointIndex - (SampleIndex_Z * SDFRes * SDFRes) - (SampleIndex_Y * SDFRes);

		return vec3(
				BOUNDINGMIN.x + (SampleIndex_X * SAMPLE_DIST.x),
				BOUNDINGMIN.y + (SampleIndex_Y * SAMPLE_DIST.y),
				BOUNDINGMIN.z + (SampleIndex_Z * SAMPLE_DIST.z));
}
void MultiThreaded_SDFSamplingNew() {
	unsigned int jobindex = processing_jobindex.fetch_add(1);
	const float sample_dist_length = length(SAMPLE_DIST);
	const float maxsampledist = length(SAMPLE_DIST) * SamplingD / 2;
	const unsigned char samplefiltercount = (maxsampledist / length(SAMPLE_DIST)) + 1;
	const bool is_lastjob = (jobindex == sdf_jobcount - 1) ? true : false;
	const unsigned int thread_jobcount = (is_lastjob) ? (sdf_lastjob_elementcount) : (sdf_elementcount_perjob);
	for (unsigned int i = 0; i < thread_jobcount; i++) {
		unsigned int mainPointIndex = (jobindex * sdf_elementcount_perjob) + i;
		glm::vec3 main_pos = PC_positions[mainPointIndex];
		glm::dvec3 pos_relativetoBB = glm::dvec3(main_pos) - BOUNDINGMIN;
		
		glm::dvec3 pos_box_frac = pos_relativetoBB / SAMPLE_DIST, pos_box_intpart;
		glm::uvec3 pos_box_indexes;
		pos_box_frac.x = modf(pos_box_frac.x, &pos_box_intpart.x); 
		pos_box_frac.y = modf(pos_box_frac.y, &pos_box_intpart.y); 
		pos_box_frac.z = modf(pos_box_frac.z, &pos_box_intpart.z);
		pos_box_indexes = pos_box_intpart;

		for (unsigned char filter_i = 0; filter_i < (samplefiltercount + 2) * (samplefiltercount + 2) * (samplefiltercount + 2); filter_i++) {
			glm::uvec3 current_sample_index;
			current_sample_index.z = filter_i / ((samplefiltercount + 2) * (samplefiltercount + 2));
			current_sample_index.y = (filter_i - (current_sample_index.z * (samplefiltercount + 2) * (samplefiltercount + 2))) / (samplefiltercount + 2);
			current_sample_index.x = filter_i - (current_sample_index.z * (samplefiltercount + 2) * (samplefiltercount + 2)) - (current_sample_index.y * (samplefiltercount + 2));

			if (pos_box_indexes.x + current_sample_index.x - samplefiltercount < 0 || pos_box_indexes.x + current_sample_index.x - samplefiltercount >= SDFRes) { continue; }
			if (pos_box_indexes.y + current_sample_index.y - samplefiltercount < 0 || pos_box_indexes.y + current_sample_index.y - samplefiltercount >= SDFRes) { continue; }
			if (pos_box_indexes.z + current_sample_index.z - samplefiltercount < 0 || pos_box_indexes.z + current_sample_index.z - samplefiltercount >= SDFRes) { continue; }
			current_sample_index = pos_box_indexes + current_sample_index - glm::uvec3(samplefiltercount);
			unsigned int current_sample_index_vector = (current_sample_index.z * SDFRes * SDFRes) + (current_sample_index.y * SDFRes) + current_sample_index.x;

			glm::vec3 sampleloc = vec3(BOUNDINGMIN.x + (current_sample_index.x * SAMPLE_DIST.x),
				BOUNDINGMIN.y + (current_sample_index.y * SAMPLE_DIST.y),
				BOUNDINGMIN.z + (current_sample_index.z * SAMPLE_DIST.z));

			vec3 Sample_toPCPoint = sampleloc - main_pos;
			float dist_fromsurface = length(Sample_toPCPoint) * dot(normalize(Sample_toPCPoint), PC_normals[mainPointIndex]);

			if (length(Sample_toPCPoint) > maxsampledist) { continue; }
			else {
				SDF_lockers[current_sample_index_vector].lock();
				if (abs(dist_fromsurface) < SDFValues[current_sample_index_vector]) {
					SDFValues[current_sample_index_vector] = dist_fromsurface;
					SDF_nearestpointindexes[current_sample_index_vector] = mainPointIndex;
				}
				SDF_lockers[current_sample_index_vector].unlock();
			}
		}
	}

	finished_jobindex.fetch_add(1);
}

void MultiThreaded_MC(){
	unsigned int jobindex = processing_jobindex.fetch_add(1);
	const float maxsampledist = length(SAMPLE_DIST) * SamplingD / 2;
	const bool is_lastjob = (jobindex == mc_jobcount - 1) ? true : false;
	const unsigned int thread_jobcount = (is_lastjob) ? mc_lastjob_elementcount : mc_elementcount_perjob;
	for (unsigned int i = 0; i < thread_jobcount; i++) {
		unsigned int CellIndex = (mc_elementcount_perjob * jobindex) + i;
		static constexpr uvec2 EdgeList[12] = {
			uvec2(0, 1), uvec2(1, 2), uvec2(2,3), uvec2(3,0),
			uvec2(4, 5), uvec2(5,6), uvec2(6,7), uvec2(7,4),
			uvec2(0, 4), uvec2(1,5), uvec2(2,6), uvec2(3,7)
		};
		unsigned int CellIndex_Z = CellIndex / ((SDFRes - 1) * (SDFRes - 1));
		unsigned int CellIndex_Y = (CellIndex - (CellIndex_Z * (SDFRes - 1) * (SDFRes - 1))) / (SDFRes - 1);
		unsigned int CellIndex_X = CellIndex - (CellIndex_Z * (SDFRes - 1) * (SDFRes - 1)) - (CellIndex_Y * (SDFRes - 1));

		unsigned int SampleIndexes[8] = { (CellIndex_Z * SDFRes * SDFRes) + (CellIndex_Y * SDFRes) + CellIndex_X,
		(CellIndex_Z * SDFRes * SDFRes) + (CellIndex_Y * SDFRes) + CellIndex_X + 1,
		((CellIndex_Z + 1) * SDFRes * SDFRes) + (CellIndex_Y * SDFRes) + CellIndex_X + 1,
		((CellIndex_Z + 1) * SDFRes * SDFRes) + (CellIndex_Y * SDFRes) + CellIndex_X,
		(CellIndex_Z * SDFRes * SDFRes) + ((CellIndex_Y + 1) * SDFRes) + CellIndex_X,
		(CellIndex_Z * SDFRes * SDFRes) + ((CellIndex_Y + 1) * SDFRes) + CellIndex_X + 1,
		((CellIndex_Z + 1) * SDFRes * SDFRes) + ((CellIndex_Y + 1) * SDFRes) + CellIndex_X + 1,
		((CellIndex_Z + 1) * SDFRes * SDFRes) + ((CellIndex_Y + 1) * SDFRes) + CellIndex_X
		};
		 
		unsigned char LookUpTable_Index = 0;
		for (unsigned char CornerIndex = 0; CornerIndex < 8; CornerIndex++) {
			if (abs(SDFValues[SampleIndexes[CornerIndex]]) != FLT_MAX &&
				SDFValues[SampleIndexes[CornerIndex]] < 0.0) { 
				LookUpTable_Index |= 1 << CornerIndex;
			}
		} 

		unsigned char Table_EdgeSearchIndex = 0;  
		while (MarchingCubes_LookUpTable[LookUpTable_Index][Table_EdgeSearchIndex] != -1) {
			unsigned char EdgeIndex = MarchingCubes_LookUpTable[LookUpTable_Index][Table_EdgeSearchIndex];
			unsigned int Sample0 = SampleIndexes[EdgeList[EdgeIndex].x], Sample1 = SampleIndexes[EdgeList[EdgeIndex].y];

			//If one of the samples is infinite, place the vertex using only non-infinite sample's distance
			if (abs(SDFValues[Sample0]) == FLT_MAX || abs(SDFValues[Sample1]) == FLT_MAX) {
				float dist = abs(SDFValues[Sample0]) == FLT_MAX ? SDFValues[Sample1] : SDFValues[Sample0];
				unsigned int ActiveSample = SDFValues[Sample0] == dist ? Sample0 : Sample1, InfiniteSample = SDFValues[Sample0] == dist ? Sample1 : Sample0;
				vec3 ActiveSamplePos = FindSampleLoc_fromSampleIndex(ActiveSample), InfiniteSamplePos = FindSampleLoc_fromSampleIndex(InfiniteSample);

				vec3 finalpos = ActiveSamplePos + (normalize(ActiveSamplePos - InfiniteSamplePos) * std::min(dist, length(ActiveSamplePos - InfiniteSamplePos)));
				MC_Results[jobindex].push_back(finalpos);
			}
			else {
				unsigned int InsideSample = SDFValues[Sample0] < SDFValues[Sample1] ? Sample0 : Sample1, OutsideSample = SDFValues[Sample0] < SDFValues[Sample1] ? Sample1 : Sample0;
				vec3 InsideSamplePos = FindSampleLoc_fromSampleIndex(InsideSample), OutsideSamplePos = FindSampleLoc_fromSampleIndex(OutsideSample);
				vec3 Inside_to_Outside = OutsideSamplePos - InsideSamplePos;

				vec3 finalpos = InsideSamplePos + (normalize(Inside_to_Outside) * (abs(SDFValues[InsideSample]) * length(Inside_to_Outside) / (abs(SDFValues[Sample0]) + abs(SDFValues[Sample1]))));
				MC_Results[jobindex].push_back(finalpos);
			}
			 
			Table_EdgeSearchIndex++;
		}
	}

	finished_jobindex.fetch_add(1);
}
extern "C" __declspec(dllexport) void* LoadPC_andReconstruct(const char* PATH, unsigned char res_multiplier, float samplingD_i, int* length, double* timing){
	auto START = std::chrono::system_clock::now();
	tapi_threadingsystem thread_sys;
    tapi_JobSystem_Start(&thread_sys);

    //Load Mesh as Point Cloud
    {
		//Check if model is available
		Assimp::Importer import;
		const aiScene* Scene = nullptr;
		{
			Scene = import.ReadFile(PATH, aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_FindInvalidData);

			//Check if scene reading errors!
			if (!Scene || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Scene->mRootNode) {
				std::cout << "Failed on Loading Mesh with Assimp; " << import.GetErrorString();
			}

			if (Scene->mNumMeshes == 0) {
				std::cout << "Failed because there is no mesh in loaded scene!";
			}
		}
        PC_pointcount = 0;
		//Importing process
		for (unsigned int i = 0; i < Scene->mNumMeshes; i++) {
			PC_pointcount += Scene->mMeshes[i]->mNumVertices;
		}
		PC_positions = new vec3[PC_pointcount];
		unsigned int StartIndex = 0;
		for (unsigned int i = 0; i < Scene->mNumMeshes; i++) {
			//First, fill the position buffer
			memcpy(&PC_positions[StartIndex], Scene->mMeshes[i]->mVertices, Scene->mMeshes[i]->mNumVertices * sizeof(vec3));
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
			PC_normals = new vec3[PC_pointcount];
			for (unsigned int i = 0; i < Scene->mNumMeshes; i++) {
				//First, fill the position buffer
				memcpy(&PC_normals[StartIndex], Scene->mMeshes[i]->mNormals, Scene->mMeshes[i]->mNumVertices * sizeof(vec3));
				StartIndex += Scene->mMeshes[i]->mNumVertices;
			}
		}
    }

    //Calculate Bounding Box
    {
        BOUNDINGMAX = dvec3(-DBL_MAX);
        BOUNDINGMIN = dvec3(DBL_MAX);
        //Calculate the center and the bounding sphere radius
        for (unsigned int i = 0; i < PC_pointcount; i++) {
            if(PC_positions[i].x < BOUNDINGMIN.x){BOUNDINGMIN.x = PC_positions[i].x;}
            if(PC_positions[i].y < BOUNDINGMIN.y){BOUNDINGMIN.y = PC_positions[i].y;}
            if(PC_positions[i].z < BOUNDINGMIN.z){BOUNDINGMIN.z = PC_positions[i].z;}

            if(PC_positions[i].x > BOUNDINGMAX.x){BOUNDINGMAX.x = PC_positions[i].x;}
            if(PC_positions[i].y > BOUNDINGMAX.y){BOUNDINGMAX.y = PC_positions[i].y;}
            if(PC_positions[i].z > BOUNDINGMAX.z){BOUNDINGMAX.z = PC_positions[i].z;}
        }
    }

    //Initiliaze variables
    {
		SDFRes = 10 * res_multiplier;
        SDFValues.clear();
        SDFValues.resize(SDFRes * SDFRes * SDFRes, FLT_MAX);
		MC_Results.clear();
        MC_Results.resize(mc_jobcount);
		SamplingD = samplingD_i;
		finished_jobindex.store(0);
		processing_jobindex.store(0);
		SDF_nearestpointindexes.clear();
        
        dvec3 CUBESIZE = BOUNDINGMAX - BOUNDINGMIN;
        BOUNDINGMAX = BOUNDINGMAX  + (CUBESIZE / 20.0);
        BOUNDINGMIN = BOUNDINGMIN - (CUBESIZE / 20.0);
        CUBESIZE *= 1.1;
        SAMPLE_DIST = CUBESIZE / dvec3(SDFRes - uvec3(1));


        sdf_elementcount_perjob = PC_pointcount / sdf_jobcount;
        sdf_lastjob_elementcount = sdf_elementcount_perjob + ((PC_pointcount) % sdf_jobcount);
        SDF_nearestpointindexes.resize(SDFRes * SDFRes * SDFRes, UINT32_MAX);
        if(SDF_lockers){ delete[] SDF_lockers; }
        SDF_lockers = new std::mutex[SDFRes * SDFRes * SDFRes];

        mc_elementcount_perjob = ((SDFRes - 1) * (SDFRes - 1) * (SDFRes - 1)) / mc_jobcount;
        mc_lastjob_elementcount = mc_elementcount_perjob + (((SDFRes - 1) * (SDFRes - 1) * (SDFRes - 1)) % mc_jobcount);
    }

    int crashing_input = 0;
    {
        for(unsigned int i = 0; i < sdf_jobcount; i++){
            tapi_Execute_withoutWait(thread_sys, MultiThreaded_SDFSamplingNew);
        }
        tapi_waitForAllOtherJobs(thread_sys);
        if(finished_jobindex.load() != sdf_jobcount){
            std::cout << "SDF Sync failed!";
            std::cin >> crashing_input;
        }
        processing_jobindex.store(0); finished_jobindex.store(0);
    }
    {
        for(unsigned int i = 0; i < mc_jobcount; i++){
            tapi_Execute_withoutWait(thread_sys, MultiThreaded_MC);
        }
        tapi_waitForAllOtherJobs(thread_sys);
        if(finished_jobindex.load() != mc_jobcount){
            std::cout << "MC Sync failed!";
            std::cin >> crashing_input;
        }
        processing_jobindex.store(0); finished_jobindex.store(0);
    }

	unsigned int VertexCount = 0;
	for(unsigned int i = 0; i < MC_Results.size(); i++) {
		VertexCount += MC_Results[i].size();
	}

	vec3* data = new vec3[VertexCount];

	unsigned int copydist_i = 0;
	for(unsigned int i = 0; i < MC_Results.size(); i++){
		memcpy(&data[copydist_i], MC_Results[i].data(), MC_Results[i].size() * sizeof(vec3));
		copydist_i += MC_Results[i].size();
	}
	*length = VertexCount;

    tapi_CloseJobSystem(thread_sys);
	auto FINISH = std::chrono::system_clock::now();
	if(timing){*timing = std::chrono::duration_cast<std::chrono::milliseconds>(FINISH - START).count();}
	return data;
}