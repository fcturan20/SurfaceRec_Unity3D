#version 460 core
in vec3 VertexNormals;
in float MAXDIST_f;
out vec4 Fragment_Color;
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

layout (std430) buffer LIGHTs{
	DirectionalLight DIRECTIONALLIGHTs[MAX_DIRECTIONALLIGHTs];
	PointLight POINTLIGHTs[MAX_POINTLIGHTs];
	SpotLight SPOTLIGHTs[MAX_SPOTLIGHTs];
	uint DIRECTIONALLIGHTs_COUNT, POINTLIGHTs_COUNT, SPOTLIGHTs_COUNT;
};

layout(binding = 0) uniform sampler2D DIFFUSETEXTURE; 
layout(binding = 1) uniform sampler2D NORMALSTEXTURE; 
layout(binding = 2) uniform sampler2D SPECULARTEXTURE; 
layout(binding = 3) uniform sampler2D OPACITYTEXTURE;
uniform uint MATINST_INDEX;

float near = 0.1f; 
float far  = 10000.0f; 
  
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

vec3 CalculateHeatMap(float minimum, float maximum, float value){
	vec3 c = vec3(0.0f, 0.0f, 0.0f);
	float ratio = 2 * (value - minimum) / (maximum - value);
	c.z = float(max(0.0f, float(1 - ratio)));
	c.x = float(max(0.0f, float(ratio - 1)));
	c.y = max(0.0f, 1 - c.x - c.z);
	return c;
}


void main(){
	//vec4 DIFFUSE = vec4((VertexNormals + 1) / 2, 1.0f);
	vec3 lightDir = DIRECTIONALLIGHTs[0].DIRECTION;
	float dotproduct = normalize(dot(normalize(lightDir), normalize(VertexNormals)));
	gl_FragDepth = LinearizeDepth(gl_FragCoord.z) / far;
	vec3 C = vec3(dotproduct);
	float gamma = 2.2f;
	Fragment_Color = vec4(pow(C, vec3(1.0f/gamma)),1.0f);
	//Fragment_Color = vec4(C, 1.0f);
}