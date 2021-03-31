#include "DijkstraSPFAlgorithm.h"
#include "TuranAPI/Profiler_Core.h"
#include "geometrycentral/surface/edge_length_geometry.h"
#include "geometrycentral/surface/flip_geodesics.h"
#include "geometrycentral/surface/manifold_surface_mesh.h"
#include "geometrycentral/surface/mesh_graph_algorithms.h"
#include "geometrycentral/surface/meshio.h"
#include "geometrycentral/surface/polygon_soup_mesh.h"
#include "geometrycentral/surface/vertex_position_geometry.h"
#include "geometrycentral/utilities/timing.h"
#include <time.h>
#include "Editor/FileSystem/DataFormats/PreProccessMesh_generated.h"

namespace TuranEditor {

	void VisualizationBuffer::GetDistances(vector<float>& Distances) {
		if (DATASIZE > 4) {
			Distances.clear();
			Distances.resize((DATASIZE / 4) - 1);
			memcpy(Distances.data(), ((char*)DATA) + 4, DATASIZE - 4);
		}
	}
	float VisualizationBuffer::GetMaxDistance() {
		return (*(float*)DATA);
	}
	void VisualizationBuffer::SetDatas(const vector<float>& Distances, const float& Max) {
		DATASIZE = (Distances.size() + 1) * 4;
		DATA = new char[DATASIZE];
		(*(float*)DATA) = Max;
		memcpy(((char*)DATA) + 4, Distances.data(), Distances.size() * 4);
	}
	const void* VisualizationBuffer::GetGPUData(unsigned int& data_size) {
		data_size = DATASIZE;
		return DATA;
	}


	void Find_AllShortestPaths(const Static_Mesh_Data& Mesh, unsigned int SourceVertexID, vector<DijkstraVertex>& Vertexes) {
		Vertexes.clear();
		if (!Mesh.VERTEX_NUMBER) {
			LOG_CRASHING("Dijkstra: There is no vertex, so FindShortestPath() didn't do anything!");
			return;
		}
		if (SourceVertexID >= Mesh.VERTEX_NUMBER) {
			LOG_CRASHING("Dijkstra: Source VertexID is bigger than total vertex count!");
			return;
		}

		Vertexes.resize(Mesh.VERTEX_NUMBER);
		for (unsigned int VertexIndex = 0; VertexIndex < Mesh.VERTEX_NUMBER; VertexIndex++) {
			Vertexes[VertexIndex].CurrentVertex = &Mesh.Vertexes[VertexIndex];
			Vertexes[VertexIndex].is_Visited = false;
			Vertexes[VertexIndex].PreviousVertex = nullptr;
			Vertexes[VertexIndex].DistanceToStart = FLT_MAX;
		}
		Vertexes[SourceVertexID].DistanceToStart = 0;

		unsigned int VertexCount = Vertexes.size();
		while (VertexCount) {
			//Because we use infinite values, we should search for the nonsentinel values first to detect which vertex to use
			DijkstraVertex* CurrentNode = nullptr;
			float MinDist = FLT_MAX;
			for (unsigned int SearchIndex = 0; SearchIndex < Vertexes.size(); SearchIndex++) {
				if (Vertexes[SearchIndex].DistanceToStart < MinDist && !Vertexes[SearchIndex].is_Visited) {
					CurrentNode = &Vertexes[SearchIndex];
					MinDist = CurrentNode->DistanceToStart;
				}
			}
			if (!CurrentNode) {
				break;
			}
			CurrentNode->is_Visited = true;
			VertexCount--;

			for (unsigned int NeighborVertexIndex = 0; NeighborVertexIndex < CurrentNode->CurrentVertex->CommonEdges.size(); NeighborVertexIndex++) {
				//LOG_STATUS("Neighbor Index: " + to_string(NeighborVertexIndex));
				//Find the edge to find neighbor
				int EdgeIndex = CurrentNode->CurrentVertex->CommonEdges[NeighborVertexIndex];
				if (EdgeIndex < 0) {
					continue;
				}
				const Edge& CurrentEdge = Mesh.Edges[EdgeIndex];
				unsigned int NeighborVertexID = (CurrentEdge.VertexIDs[0] == CurrentNode->CurrentVertex->VertexID) ? CurrentEdge.VertexIDs[1] : CurrentEdge.VertexIDs[0];
				//LOG_STATUS("Neighbor VertexIndex: " + to_string(NeighborVertexID));
				DijkstraVertex* NeighborVertex = &Vertexes[NeighborVertexID];
				if (NeighborVertex->is_Visited) {
					continue;
				}

				float NewPathDistance = CurrentNode->DistanceToStart + CurrentEdge.Length;

				if (NewPathDistance < NeighborVertex->DistanceToStart) {
					NeighborVertex->DistanceToStart = NewPathDistance;
					NeighborVertex->PreviousVertex = CurrentNode;
					//LOG_STATUS("NewPath is set!");
				}
			}
		}
	}
	void FindShortestPath(const Static_Mesh_Data& Mesh, unsigned int StartVertexID, unsigned int TargetVertexID, vector<unsigned int>& Path) {
		TURAN_PROFILE_SCOPE("Shortest Path ");
		Path.clear();
		vector<DijkstraVertex> Paths;
		Find_AllShortestPaths(Mesh, StartVertexID, Paths);
		unsigned int CurrentVertexID = Paths[TargetVertexID].CurrentVertex->VertexID;
		while (CurrentVertexID != StartVertexID) {
			Path.push_back(CurrentVertexID);
			CurrentVertexID = Paths[CurrentVertexID].PreviousVertex->CurrentVertex->VertexID;
		}
		Path.push_back(StartVertexID);
	}
	float FindShortestDistance(const Static_Mesh_Data& Mesh, unsigned int StartVertexID, unsigned int TargetVertexID) {
		vector<DijkstraVertex> Paths;
		Find_AllShortestPaths(Mesh, StartVertexID, Paths);
		return Paths[TargetVertexID].DistanceToStart;
	}
	void FindAllShortestDistances(const Static_Mesh_Data& Mesh, unsigned int StartVertexID, vector<float>& Distances) {
		vector<DijkstraVertex> Paths;
		Find_AllShortestPaths(Mesh, StartVertexID, Paths);
		Distances.clear();
		for (unsigned int i = 0; i < Paths.size(); i++) {
			Distances.push_back(Paths[i].DistanceToStart);
		}
	}
	void GetVisData_FromDijkstraVertexes(const vector<DijkstraVertex>& Vertexes, VisualizationBuffer& VisBuf) {
		vector<float> Distances;	Distances.resize(Vertexes.size());
		float MAXDIST = FLT_MIN;
		for (unsigned int VertexElement = 0; VertexElement < Vertexes.size(); VertexElement++) {
			unsigned int CurrentID = Vertexes[VertexElement].CurrentVertex->VertexID;
			Distances[CurrentID] = Vertexes[VertexElement].DistanceToStart;
			if (Distances[CurrentID] > MAXDIST) {
				MAXDIST = Distances[CurrentID];
			}
		}

		VisBuf.SetDatas(Distances, MAXDIST);
	}

	void FPS_on_Mesh(const Static_Mesh_Data& Mesh, unsigned int SampleCount, vector<unsigned int>& SampleIndexes) {
		TURAN_PROFILE_SCOPE("FPS on Mesh!");
		SampleIndexes.clear();
		if (SampleCount < 1 || SampleCount > Mesh.VERTEX_NUMBER) {
			LOG_CRASHING("You should fix the Sample Count, either less than 1 or bigger than Vertex Count");
			return;
		}
		srand(time(0));
		unsigned int FirstSample = rand() % Mesh.VERTEX_NUMBER;
		SampleIndexes.push_back(FirstSample);

		vector<vector<float>> GeodesicsDistances;
		while (SampleIndexes.size() != SampleCount) {
			//Find geodesics of the new sample
			GeodesicsDistances.push_back(vector<float>());
			unsigned int LastSample = SampleIndexes.size() - 1;
			FindAllShortestDistances(Mesh, SampleIndexes[LastSample], GeodesicsDistances[LastSample]);

			//Link each vertex to the closest sample point
			float FurthestDistance = FLT_MIN;
			unsigned int NewSampleVertexID = 0;
			vector<unsigned int> AssociatedSample_perVertex;  //Each element index means each vertex, their value means which of them they are associated with!
			for (unsigned int VertexIndex = 0; VertexIndex < Mesh.VERTEX_NUMBER; VertexIndex++) {
				float geodistance = FLT_MAX;
				unsigned int LinkedSample = 0;
				for (unsigned int SampleIndex = 0; SampleIndex < SampleIndexes.size(); SampleIndex++) {
					if (GeodesicsDistances[SampleIndex][VertexIndex] < geodistance) {
						LinkedSample = SampleIndex;
						geodistance = GeodesicsDistances[SampleIndex][VertexIndex];
					}
				}
				if (geodistance == FLT_MAX) {
					LOG_WARNING("There are some discontinuities on the mesh while sampling points!");
					continue;
				}
				AssociatedSample_perVertex.push_back(LinkedSample);
				//If the vertex's distance to the associated sample point is greater than current FurthestDistance, this vertex should be selected
				if (geodistance > FurthestDistance) {
					FurthestDistance = geodistance;
					NewSampleVertexID = VertexIndex;
				}
			}
			if (FurthestDistance == FLT_MIN) {
				LOG_CRASHING("New sample point can't be found because there is no any other vertex that can be continued to these vertexes, I think!");
				return;
			}
			SampleIndexes.push_back(NewSampleVertexID);
		}



	}


	void PairWiseDistanceMatrix::Construct(vector<vector<float>> GeodesicDistances, vector<unsigned int> SampledVertexIDs) {
		MatrixData.resize(SampledVertexIDs.size() * SampledVertexIDs.size());
		for (unsigned int ColumnIndex = 0; ColumnIndex < SampledVertexIDs.size(); ColumnIndex++) {
			for (unsigned int RowIndex = 0; RowIndex < SampledVertexIDs.size(); RowIndex++) {
				MatrixData[SampledVertexIDs.size() * RowIndex + ColumnIndex] = GeodesicDistances[ColumnIndex][SampledVertexIDs[RowIndex]];
			}
		}
	}



	void PreprocessedMesh::PreProcessMesh(const Static_Mesh_Data& Mesh, unsigned int SampleCount) {
		SampledVertexIDs.clear();
		if (SampleCount < 1 || SampleCount > Mesh.VERTEX_NUMBER) {
			LOG_CRASHING("You should fix the Sample Count, either less than 1 or bigger than Vertex Count");
			return;
		}
		srand(time(0));
		unsigned int FirstSample = rand() % Mesh.VERTEX_NUMBER;
		SampledVertexIDs.push_back(FirstSample);

		vector<vector<float>> GeodesicsDistances;
		while (SampledVertexIDs.size() != SampleCount) {
			//Find geodesics of the new sample
			GeodesicsDistances.push_back(vector<float>());
			unsigned int LastSample = SampledVertexIDs.size() - 1;
			FindAllShortestDistances(Mesh, SampledVertexIDs[LastSample], GeodesicsDistances[LastSample]);

			//Link each vertex to the closest sample point
			float FurthestDistance = FLT_MIN;
			unsigned int NewSampleVertexID = 0;
			vector<unsigned int> AssociatedSample_perVertex;  //Each element index means each vertex, their value means which of them they are associated with!
			for (unsigned int VertexIndex = 0; VertexIndex < Mesh.VERTEX_NUMBER; VertexIndex++) {
				float geodistance = FLT_MAX;
				unsigned int LinkedSample = 0;
				for (unsigned int SampleIndex = 0; SampleIndex < SampledVertexIDs.size(); SampleIndex++) {
					if (GeodesicsDistances[SampleIndex][VertexIndex] < geodistance) {
						LinkedSample = SampleIndex;
						geodistance = GeodesicsDistances[SampleIndex][VertexIndex];
					}
				}
				if (geodistance == FLT_MAX) {
					LOG_WARNING("There are some discontinuities on the mesh while sampling points!");
					continue;
				}
				AssociatedSample_perVertex.push_back(LinkedSample);
				//If the vertex's distance to the associated sample point is greater than current FurthestDistance, this vertex should be selected
				if (geodistance > FurthestDistance) {
					FurthestDistance = geodistance;
					NewSampleVertexID = VertexIndex;
				}
			}
			if (FurthestDistance == FLT_MIN) {
				LOG_CRASHING("New sample point can't be found because there is no any other vertex that can be continued to these vertexes, I think!");
				return;
			}
			SampledVertexIDs.push_back(NewSampleVertexID);
		}

		//Find geodesics of the last sample
		GeodesicsDistances.push_back(vector<float>());
		unsigned int LastSample = SampledVertexIDs.size() - 1;
		FindAllShortestDistances(Mesh, SampledVertexIDs[LastSample], GeodesicsDistances[LastSample]);

		//Find all 3 nearest FPSes for each vertex and store them
		PPedVertexData.clear();
		for (unsigned int VertexIndex = 0; VertexIndex < Mesh.VERTEX_NUMBER; VertexIndex++) {
			bool is_samplepoint = false;
			for (unsigned int SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++) {
				if (VertexIndex == SampledVertexIDs[SampleIndex]) {
					is_samplepoint = true;
					break;
				}
			}
			if (is_samplepoint) {
				continue;
			}
			PPedVertexData.push_back(PPVertexData());
			PPVertexData& PP = PPedVertexData[PPedVertexData.size() - 1];
			PP.VertexID = VertexIndex;
			float Nearest = PP.GeodesicDistances[0];
			for (unsigned int SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++) {
				float Geodesic = GeodesicsDistances[SampleIndex][VertexIndex];
				if (Geodesic < Nearest) {
					PP.GeodesicDistances[0] = Geodesic;
					PP.GeodesicVertexes[0] = SampleIndex;
					Nearest = Geodesic;
				}
			}
			Nearest = FLT_MAX;
			for (unsigned int SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++) {
				if (PP.GeodesicVertexes[0] == SampleIndex) {
					continue;
				}
				float Geodesic = GeodesicsDistances[SampleIndex][VertexIndex];
				if (Geodesic < Nearest) {
					PP.GeodesicDistances[1] = Geodesic;
					PP.GeodesicVertexes[1] = SampleIndex;
					Nearest = Geodesic;
				}
			}
			Nearest = FLT_MAX;
			for (unsigned int SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++) {
				if (PP.GeodesicVertexes[0] == SampleIndex || PP.GeodesicVertexes[1] == SampleIndex) {
					continue;
				}
				float Geodesic = GeodesicsDistances[SampleIndex][VertexIndex];
				if (Geodesic < Nearest) {
					PP.GeodesicDistances[2] = Geodesic;
					PP.GeodesicVertexes[2] = SampleIndex;
					Nearest = Geodesic;
				}
			}
		}
		SampleDistances.Construct(GeodesicsDistances, SampledVertexIDs);
	}
	void PreprocessedMesh::StorePreProcessedData(const char* PATH_wName_wExtension) {
		flatbuffers::FlatBufferBuilder build(1024);

		vector<VertexData> CompiledVertexData;
		for (unsigned int VertexIndex = 0; VertexIndex < PPedVertexData.size(); VertexIndex++) {
			CompiledVertexData.push_back(VertexData(PPedVertexData[VertexIndex].VertexID, PPedVertexData[VertexIndex].GeodesicDistances[0], PPedVertexData[VertexIndex].GeodesicVertexes[0],
				PPedVertexData[VertexIndex].GeodesicDistances[1], PPedVertexData[VertexIndex].GeodesicVertexes[1],
				PPedVertexData[VertexIndex].GeodesicDistances[2], PPedVertexData[VertexIndex].GeodesicVertexes[2]));
		}
		
		auto CompiledGeodesic = CreateGeodesicResourceDirect(build, &SampleDistances.MatrixData, &SampledVertexIDs, &CompiledVertexData);
		build.Finish(CompiledGeodesic);

		void* data_ptr = build.GetBufferPointer();
		unsigned int data_size = build.GetSize();


		//Check if the data is complete!
		flatbuffers::Verifier verifier((uint8_t*)data_ptr, data_size);
		if (!VerifyGeodesicResourceBuffer(verifier)) {
			LOG_CRASHING("Verification of the PreprocessedMesh has failed!\n");
		}
		TAPIFILESYSTEM::Write_BinaryFile(PATH_wName_wExtension, data_ptr, data_size);
	}
	void PreprocessedMesh::LoadFromDisk(const char* PATH_wName_wExtension) {
		unsigned int datasize = 0;
		void* bdata = TAPIFILESYSTEM::Read_BinaryFile(PATH_wName_wExtension, datasize);

		auto GeodesicFile = GetGeodesicResource(bdata);
		SampleDistances.MatrixData.clear();	
		SampledVertexIDs.clear();
		PPedVertexData.clear();
		SampleDistances.MatrixData.resize(GeodesicFile->FPSDistances()->size());
		SampledVertexIDs.resize(GeodesicFile->FPSVertexIndexes()->size());
		PPedVertexData.resize(GeodesicFile->VertexGeodesics()->size());

		for (unsigned int pwFPSIndex = 0; pwFPSIndex < SampleDistances.MatrixData.size(); pwFPSIndex++) {
			SampleDistances.MatrixData[pwFPSIndex] = GeodesicFile->FPSDistances()->Get(pwFPSIndex);
		}
		for (unsigned int FPSIndex = 0; FPSIndex < SampledVertexIDs.size(); FPSIndex++) {
			SampledVertexIDs[FPSIndex] = GeodesicFile->FPSVertexIndexes()->Get(FPSIndex);
		}
		for (unsigned int VertexIndex = 0; VertexIndex < GeodesicFile->VertexGeodesics()->size(); VertexIndex++) {
			const VertexData* readDATA = GeodesicFile->VertexGeodesics()->Get(VertexIndex);
			PPVertexData& FillData = PPedVertexData[VertexIndex];

			FillData.VertexID = readDATA->VertexID();
			FillData.GeodesicDistances[0] = readDATA->GeodesicVertex0Distance();
			FillData.GeodesicDistances[1] = readDATA->GeodesicVertex1Distance();
			FillData.GeodesicDistances[2] = readDATA->GeodesicVertex2Distance();

			FillData.GeodesicVertexes[0] = readDATA->GeodesicVertex0SampleID();
			FillData.GeodesicVertexes[1] = readDATA->GeodesicVertex1SampleID();
			FillData.GeodesicVertexes[2] = readDATA->GeodesicVertex2SampleID();
		}
		delete bdata;
	}
	void PreprocessedMesh::PrintData() {
		for (unsigned int VertexIndex = 0; VertexIndex < PPedVertexData.size(); VertexIndex++) {
			PPVertexData& PP = PPedVertexData[VertexIndex];
			//Print the results
			for (unsigned char printindex = 0; printindex < 3; printindex++) {
				std::cout << "Print VertexID: " << VertexIndex << " Sample: " << unsigned int(printindex) << " Vector Index: " << PP.GeodesicVertexes[printindex] << " Vertex ID: " << SampledVertexIDs[PP.GeodesicVertexes[printindex]] << " and Distance: " << PP.GeodesicDistances[printindex] << std::endl;
			}
		}
	}
	void PreprocessedMesh::GetVisualizationBufferData(unsigned int SourceVertexID, VisualizationBuffer& VisBuf) {
		unsigned int VertexCount = PPedVertexData.size() + SampledVertexIDs.size();
		if (SourceVertexID >= VertexCount) {
			LOG_CRASHING("You give a wrong SourceVertexID to GetVisualizationBufferData()");
			return;
		}

		vector<float> PerVertexDistance;
		PerVertexDistance.resize(PPedVertexData.size() + SampledVertexIDs.size());

		bool isSourceFPS = false;
		unsigned int SourceFPSElementIndex = SampledVertexIDs.size();
		for (unsigned int MainSearchIndex = 0; MainSearchIndex < SampledVertexIDs.size(); MainSearchIndex++) {
			if (SampledVertexIDs[MainSearchIndex] == SourceVertexID) {
				isSourceFPS = true;
				SourceFPSElementIndex = MainSearchIndex;
				break;
			}
		}
		for (unsigned int VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++) {
			if (SourceVertexID == VertexIndex) {
				PerVertexDistance[VertexIndex] = 0.0f;
				continue;
			}
			//Search if current vertex is an FPS point. If it is;
			//1) Source is FPS too, so read the preprocessed data
			//2) Source is a vertex, so calculate
			bool isCurrentFPS = false;
			for (unsigned int FPSElementIndex = 0; FPSElementIndex < SampledVertexIDs.size(); FPSElementIndex++) {
				if (SampledVertexIDs[FPSElementIndex] == VertexIndex) {
					isCurrentFPS = true;
					if (isSourceFPS) {
						PerVertexDistance[VertexIndex] = SampleDistances.MatrixData[FPSElementIndex * SampledVertexIDs.size() + SourceFPSElementIndex];
					}
					else {
						PPVertexData& Data = FindPPedVERTEXID(SourceVertexID);
						float LEASTDIST = FLT_MAX;
						for (unsigned int SourceGVertexIndex = 0; SourceGVertexIndex < 3; SourceGVertexIndex++) {
							float SVERTEX_to_SGVERTEX = Data.GeodesicDistances[SourceGVertexIndex];
							unsigned int GVERTEXELEMENT = Data.GeodesicVertexes[SourceGVertexIndex];
							float currentdist = SVERTEX_to_SGVERTEX + SampleDistances.MatrixData[GVERTEXELEMENT * SampledVertexIDs.size() + FPSElementIndex];
							if (currentdist < LEASTDIST) {
								LEASTDIST = currentdist;
							}
						}
						if (LEASTDIST != FLT_MAX) {
							PerVertexDistance[VertexIndex] = LEASTDIST;
						}
					}
					break;
				}
			}

			if (isCurrentFPS) {
				continue;
			}
			
			//Current Vertex is not a FPS.

			if (isSourceFPS) {
				PPVertexData& Data = FindPPedVERTEXID(VertexIndex);
				float LEASTDIST = FLT_MAX;
				for (unsigned int TargetGVertexIndex = 0; TargetGVertexIndex < 3; TargetGVertexIndex++) {
					float TVERTEX_to_GVERTEX = Data.GeodesicDistances[TargetGVertexIndex];
					unsigned int TGVERTEXELEMENT = Data.GeodesicVertexes[TargetGVertexIndex];
					float currentdistance = TVERTEX_to_GVERTEX + SampleDistances.MatrixData[TGVERTEXELEMENT * SampledVertexIDs.size() + SourceFPSElementIndex];
					if (currentdistance < LEASTDIST) {
						LEASTDIST = currentdistance;
					}
				}
				if (LEASTDIST != FLT_MAX) {
					PerVertexDistance[VertexIndex] = LEASTDIST;
				}
			}
			else {
				PPVertexData& SData = FindPPedVERTEXID(SourceVertexID);
				PPVertexData& TData = FindPPedVERTEXID(VertexIndex);
				float LEASTDIST = FLT_MAX;
				for (unsigned int SGVertexIndex = 0; SGVertexIndex < 3; SGVertexIndex++) {
					float SVERTEX_to_GVERTEX = SData.GeodesicDistances[SGVertexIndex];
					unsigned int SGVERTEXELEMENT = SData.GeodesicVertexes[SGVertexIndex];
					for (unsigned int TGVertexIndex = 0; TGVertexIndex < 3; TGVertexIndex++) {
						float TVERTEX_to_GVERTEX = TData.GeodesicDistances[TGVertexIndex];
						unsigned int TGVERTEXELEMENT = TData.GeodesicVertexes[TGVertexIndex];
						float currentdist = TVERTEX_to_GVERTEX + SampleDistances.MatrixData[TGVERTEXELEMENT * SampledVertexIDs.size() + SGVERTEXELEMENT] + SVERTEX_to_GVERTEX;
						if (currentdist < LEASTDIST) {
							LEASTDIST = currentdist;
						}
					}
					if (LEASTDIST != FLT_MAX) {
						PerVertexDistance[VertexIndex] = LEASTDIST;
					}
				}
			}
		}

		float MAXDIST = FLT_MIN;
		for (unsigned int VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++) {
			if (PerVertexDistance[VertexIndex] > MAXDIST) {
				MAXDIST = PerVertexDistance[VertexIndex];
			}
		}

		VisBuf.SetDatas(PerVertexDistance, MAXDIST);
	}
	PPVertexData& PreprocessedMesh::FindPPedVERTEXID(unsigned int VertexID) {
		for (unsigned int VertexIndex = 0; VertexIndex < PPedVertexData.size(); VertexIndex++) {
			if (PPedVertexData[VertexIndex].VertexID == VertexID) {
				return PPedVertexData[VertexIndex];
			}
		}
		LOG_CRASHING("Wrong find index in FindPPedVERTEXID(): " + to_string(VertexID));
		return PPedVertexData[0];
	}

	
	vector<vec3> Find_FlipGeodesics(const char* PATH, unsigned int SourceVertexIndex, unsigned int TargetVertexIndex) {
		// Load a mesh
		std::unique_ptr<geometrycentral::surface::ManifoldSurfaceMesh> mesh;
		std::unique_ptr<geometrycentral::surface::VertexPositionGeometry> geometry;
		std::tie(mesh, geometry) = geometrycentral::surface::readManifoldSurfaceMesh(PATH);

		// Create a path network as a Dijkstra path between endpoints
		std::unique_ptr<geometrycentral::surface::FlipEdgeNetwork> edgeNetwork;
		geometrycentral::surface::Vertex vStart = mesh->vertex(SourceVertexIndex);
		geometrycentral::surface::Vertex vEnd = mesh->vertex(TargetVertexIndex);
		edgeNetwork = geometrycentral::surface::FlipEdgeNetwork::constructFromDijkstraPath(*mesh, *geometry, vStart, vEnd);

		// Make the path a geodesic
		edgeNetwork->iterativeShorten();

		// Extract the result as a polyline along the surface
		edgeNetwork->posGeom = geometry.get();
		vector<geometrycentral::Vector3> Path = edgeNetwork->getPathPolyline3D()[0];
		vector<vec3> PathFinal;
		PathFinal.resize(Path.size());
		for (unsigned int PathVertex = 0; PathVertex < Path.size(); PathVertex++) {
			PathFinal[PathVertex].x = Path[PathVertex][0];
			PathFinal[PathVertex].y = Path[PathVertex][1];
			PathFinal[PathVertex].z = Path[PathVertex][2];
		}
		return PathFinal;
	}
	//
	vector<vec3> Find_DijkstraPathLoop(const char* PATH, const vector<unsigned int>& ListofVertexes, bool FlipGeodesics) {
		// Load a mesh
		std::unique_ptr<geometrycentral::surface::ManifoldSurfaceMesh> mesh;
		std::unique_ptr<geometrycentral::surface::VertexPositionGeometry> geometry;
		std::tie(mesh, geometry) = geometrycentral::surface::readManifoldSurfaceMesh(PATH);

		// Create a path network and set vertexes
		std::unique_ptr<geometrycentral::surface::FlipEdgeNetwork> edgeNetwork;
		std::vector<geometrycentral::surface::Vertex> fancyPathVerts;
		for (unsigned int i = 0; i < ListofVertexes.size(); i++) {
			fancyPathVerts.push_back(mesh->vertex(ListofVertexes[i]));
		}
		edgeNetwork = geometrycentral::surface::FlipEdgeNetwork::constructFromPiecewiseDijkstraPath(*mesh, *geometry, fancyPathVerts, true, false);
		if (!edgeNetwork) {
			LOG_CRASHING("Edge Network has failed at creation!");
			return vector<vec3>();
		}
		if (FlipGeodesics) {
			edgeNetwork->iterativeShorten();
		}

		// Extract the result as a polyline along the surface
		edgeNetwork->posGeom = geometry.get();
		vector<geometrycentral::Vector3> Path = edgeNetwork->getPathPolyline3D()[0];
		vector<vec3> PathFinal;
		PathFinal.resize(Path.size());
		for (unsigned int PathVertex = 0; PathVertex < Path.size(); PathVertex++) {
			PathFinal[PathVertex].x = Path[PathVertex][0];
			PathFinal[PathVertex].y = Path[PathVertex][1];
			PathFinal[PathVertex].z = Path[PathVertex][2];
		}
		return PathFinal;
	}
}