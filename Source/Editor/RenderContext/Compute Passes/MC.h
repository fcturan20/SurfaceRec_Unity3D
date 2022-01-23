#include "Editor/Editor_Includes.h"
#include "GFX/GFX_Includes.h"
#include "GFX/Renderer/GFX_Resource.h"
#include "GFX/Renderer/GFX_RenderGraph.h"



class MC_ComputePass : public GFX_API::ComputePass {
	virtual void Execute() override;
	bool* finish_flag = nullptr;
public:
	MC_ComputePass();
	void RunMC(std::vector<float> SDFSamples, dvec3 BOUNDINGMIN, dvec3 BOUNDINGMAX, float samplingD);
};