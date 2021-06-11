#version 460
layout(location = 0) in vec3 Vertex_Pos;
layout(location = 1) in vec3 Vertex_Normal;

#define MAX_WORLDOBJECTNUMBER 1000
#define MAX_SPOTLIGHTs 10
#define MAX_POINTLIGHTs 10
#define MAX_DIRECTIONALLIGHTs 1


layout (std430, binding = 0) buffer VIEW_MATRICES{
	mat4 PROJ_MATRIX;
	mat4 VIEW_MATRIX;
};

layout (std430, binding = 1) buffer MODEL_MATRICES{
	mat4 MODEL_MATRICes[MAX_WORLDOBJECTNUMBER];
};

struct PointLight{
	vec3 POSITION;
};

struct SpotLight{
	vec3 POSITION;
};

struct DirectionalLight{
	vec3 DIRECTION;
	vec3 COLOR;
};

layout (std430, binding = 2) buffer LIGHTs{
	DirectionalLight DIRECTIONALLIGHTs[MAX_DIRECTIONALLIGHTs];
	PointLight POINTLIGHTs[MAX_POINTLIGHTs];
	SpotLight SPOTLIGHTs[MAX_SPOTLIGHTs];
	uint DIRECTIONALLIGHTs_COUNT, POINTLIGHTs_COUNT, SPOTLIGHTs_COUNT;
};

//If you want to show normals, this is for the geometry shader
out vec3 vertex_normal;

uniform uint ShowNormal;

void main(){
	vertex_normal = Vertex_Normal;
	gl_Position = vec4(Vertex_Pos, 1.0f);
	if(ShowNormal == 1 && length(vertex_normal) != 0.0f){
		gl_PointSize = 2;
	}
	else{
		gl_PointSize = 10;
	}
}