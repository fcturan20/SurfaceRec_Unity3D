#version 460
out vec4 Fragment_Color;
layout (depth_less) out float gl_FragDepth;
#define MAX_MATINSTNUMBER 1000
#define MAX_SPOTLIGHTs 10
#define MAX_POINTLIGHTs 10
#define MAX_DIRECTIONALLIGHTs 1

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


in vec3 vertex_color;

void main(){
	Fragment_Color = vec4(vertex_color, 1.0f);
}