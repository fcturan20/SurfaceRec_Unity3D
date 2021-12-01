#version 460 core
in vec3 VertexNormal;
in vec3 VertexColor;
out vec4 Fragment_Color;

#define MAX_DIRECTIONALLIGHTs 1


struct DirectionalLight{
	vec3 DIRECTION;
	vec3 COLOR;
};

layout (std430) buffer LIGHTs{
	DirectionalLight DIRECTIONALLIGHT;
};

float near = 0.1f; 
float far  = 10000.0f;
  
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

uniform uint isPhongShadingActive;
uniform vec3 SurfaceColor;

void main(){
	if(isPhongShadingActive == 1){
		vec3 lightDir = DIRECTIONALLIGHT.DIRECTION;
		float dotproduct = dot(normalize(lightDir), normalize(VertexNormal));
		Fragment_Color = vec4(vec3(dotproduct), 1.0f);
	}
	else if(isPhongShadingActive == 2){
		Fragment_Color = vec4((VertexNormal + 1.0f) / 2.0f, 1.0f);
	}
	else{
		Fragment_Color = vec4(VertexColor, 1.0f);
	}
	//Fragment_Color = vec4((VertexNormal + vec3(1.0f) / vec3(2.0f)), 1.0f);
}