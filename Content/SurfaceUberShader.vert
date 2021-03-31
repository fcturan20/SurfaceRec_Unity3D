#version 460 core
layout(location = 0) in vec3 Vertex_Pos;
layout(location = 1) in vec3 Vertex_Nor;


out vec2 TextureCoords;
out vec3 VertexNormals;
out float Distance;
out float MAXDIST_f;

#define MAX_WORLDOBJECTNUMBER 1000

layout (std430) buffer VIEW_MATRICES{
	mat4 PROJ_MATRIX;
	mat4 VIEW_MATRIX;
};

layout (std140) buffer MODEL_MATRICES{
	mat4 MODEL_MATRICes[MAX_WORLDOBJECTNUMBER];
};

layout (std140) buffer GEOs{
	float MAXDIST;
	vec4 GeodesicDistance[7500];
};


//Each shader instance will only specify this for vertex shader!
//uniform uint OBJECT_INDEX;

void main(){
	gl_Position = PROJ_MATRIX * VIEW_MATRIX * MODEL_MATRICes[0] * vec4(Vertex_Pos, 1.0f);
	VertexNormals = mat3(transpose(inverse(MODEL_MATRICes[0]))) *  Vertex_Nor;
}