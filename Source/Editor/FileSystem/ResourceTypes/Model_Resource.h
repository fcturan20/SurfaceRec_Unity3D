#pragma once
#include "Editor/Editor_Includes.h"
#include "GFX/Renderer/GPU_ContentManager.h"

enum class RESOURCETYPEs : char;

namespace TuranEditor {
	
	struct Vertex {
		int VertexID;
		vec3 Position;
		vector<unsigned int> CommonEdges;
		vector<unsigned int> CommonTriangles;
	};

	struct Edge {
		uint32 VertexIDs[2]{UINT32_MAX, UINT32_MAX };
		uint32 TriangleIDs[2]{UINT32_MAX, UINT32_MAX };
		float Length = FLT_MIN;
	};

	struct Triangle {
		uint32 VertexIDs[3]{UINT32_MAX, UINT32_MAX, UINT32_MAX };
		uint32 EdgeIDs[3]{ UINT32_MAX, UINT32_MAX, UINT32_MAX };
	};
	
	struct Static_Mesh_Data {
	public:
		Static_Mesh_Data();
		~Static_Mesh_Data();

		void* VERTEX_DATA = nullptr;
		unsigned int VERTEXDATA_SIZE, VERTEX_NUMBER = 0;
		unsigned int* INDEX_DATA = nullptr;
		unsigned int INDICES_NUMBER = 0;
		unsigned short Material_Index = 0;
		
		Vertex* Vertexes;
		vector<Edge> Edges;
		Triangle* Triangles;
		
		

		GFX_API::VertexAttributeLayout DataLayout;

		bool Verify_Mesh_Data();
	};


	class Static_Model {
	public:
		Static_Model();
		~Static_Model();

		//Meshes will be stored as pointers in an array, so point to that "Pointer Array"
		vector<Static_Mesh_Data*> MESHes;
		vector<unsigned int> MATINST_IDs;
		unsigned int* WORLD_ID = nullptr;

		//Return vector contains MeshBufferIDs of the Model!
		vector<unsigned int> Upload_toGPU();
	};
}