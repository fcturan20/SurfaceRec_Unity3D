#pragma once
#include "Editor/FileSystem/ResourceTypes/Model_Resource.h"

namespace TuranEditor {
	struct VisualizationBuffer {
	private:
		void* DATA; unsigned int DATASIZE = 0;

	public:
		void GetDistances(vector<float>& Distances);
		float GetMaxDistance();
		void SetDatas(const vector<float>& Distances, const float& Max);
		const void* GetGPUData(unsigned int& data_size);
	};
	struct DijkstraVertex {
		Vertex* CurrentVertex;
		DijkstraVertex* PreviousVertex = nullptr;
		float DistanceToStart = FLT_MAX;
		bool is_Visited = false;
	};
	struct PPVertexData {
		unsigned int VertexID;
		float GeodesicDistances[3]{FLT_MAX, FLT_MAX, FLT_MAX};
		unsigned int GeodesicVertexes[3]{UINT32_MAX, UINT32_MAX, UINT32_MAX};	//These are the indexes of the elements in the SampledVertexIDs, not their values
	};
	struct PairWiseDistanceMatrix {
		void Construct(vector<vector<float>> GeodesicDistances, vector<unsigned int> SampledVertexIDs);
		vector<float> MatrixData;
		//Add a read-only operator overload here
	};
	struct PreprocessedMesh {
		void PreProcessMesh(const Static_Mesh_Data& Mesh, unsigned int SampleCount);
		void StorePreProcessedData(const char* PATH_wName_wExtension);
		void LoadFromDisk(const char* PATH_wName_wExtension);
		void PrintData();
		void GetVisualizationBufferData(unsigned int SourceVertexID, VisualizationBuffer& Buffer);
	private:
		vector<unsigned int> SampledVertexIDs;
		PairWiseDistanceMatrix SampleDistances;
		vector<PPVertexData> PPedVertexData;
		//Find farthest points and their geodesic distances to every other vertex
		//Then fill the VertexData to save it to disk later
		PPVertexData& FindPPedVERTEXID(unsigned int VertexID);
	};
	float FindShortestDistance(const Static_Mesh_Data& Mesh, unsigned int StartVertexID, unsigned int TargetVertexID);
	void FindShortestPath(const Static_Mesh_Data& Mesh, unsigned int StartVertexID, unsigned int TargetVertexID, vector<unsigned int>& Path);
	void Find_AllShortestPaths(const Static_Mesh_Data& Mesh, unsigned int SourceVertexID, vector<DijkstraVertex>& Vertexes);
	void GetVisData_FromDijkstraVertexes(const vector<DijkstraVertex>& Vertexes, VisualizationBuffer& VisBuf);
	void FPS_on_Mesh(const Static_Mesh_Data& Mesh, unsigned int SampleCount, vector<unsigned int>& SampleIndexes);
	vector<vec3> Find_FlipGeodesics(const char* PATH, unsigned int SourceVertexIndex, unsigned int TargetVertexIndex);
	vector<vec3> Find_DijkstraPathLoop(const char* PATH, const vector<unsigned int>& ListofVertexes, bool FlipGeodesics);
}
