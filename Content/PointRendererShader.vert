#version 460
layout(location = 0) in vec3 Vertex_Pos;

#define MAX_WORLDOBJECTNUMBER 1000
#define MAX_SPOTLIGHTs 10
#define MAX_POINTLIGHTs 10
#define MAX_DIRECTIONALLIGHTs 1


layout (std430) buffer VIEW_MATRICES{
	mat4 PROJ_MATRIX;
	mat4 VIEW_MATRIX;
};

layout (std430) buffer MODEL_MATRICES{
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

layout (std430) buffer LIGHTs{
	DirectionalLight DIRECTIONALLIGHTs[MAX_DIRECTIONALLIGHTs];
	PointLight POINTLIGHTs[MAX_POINTLIGHTs];
	SpotLight SPOTLIGHTs[MAX_SPOTLIGHTs];
	uint DIRECTIONALLIGHTs_COUNT, POINTLIGHTs_COUNT, SPOTLIGHTs_COUNT;
};

//Each shader instance will only specify this for vertex shader!
uniform uint OBJECT_INDEX, POINTLIGHT_INDEX, SPOTLIGHT_INDEX;
uniform float POINT_SIZE;

void main(){
	if(POINTLIGHT_INDEX > 0){
		gl_Position = PROJ_MATRIX * VIEW_MATRIX * vec4(POINTLIGHTs[POINTLIGHT_INDEX].POSITION, 1.0f);
		//gl_PointSize = POINT_SIZE;
	}
	else if (SPOTLIGHT_INDEX > 0){
		gl_Position = PROJ_MATRIX * VIEW_MATRIX * vec4(SPOTLIGHTs[SPOTLIGHT_INDEX].POSITION, 1.0f);
		//gl_PointSize = POINT_SIZE;
	}
	else{
		gl_Position = PROJ_MATRIX * VIEW_MATRIX * MODEL_MATRICes[OBJECT_INDEX] * vec4(Vertex_Pos, 1.0f);
		//gl_PointSize = 10;
	}
}