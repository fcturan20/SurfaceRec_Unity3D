#version 460 core
layout(location = 0) in vec3 Vertex_Pos;
layout(location = 1) in vec3 Vertex_Nor;
out vec3 VertexNormal;

layout (std430) buffer VIEW_MATRICES{
	mat4 PROJ_MATRIX;
	mat4 VIEW_MATRIX;
};

uniform uint isPhongShadingActive;
out vec3 VertexColor;

void main(){
	gl_Position = PROJ_MATRIX * VIEW_MATRIX * vec4(Vertex_Pos, 1.0f);
	VertexNormal = Vertex_Nor;
	if(isPhongShadingActive == 0) { 
		if(gl_VertexID % 3 == 2){VertexColor = vec3(0.0, 0.0, 1.0);}
		else if(gl_VertexID % 3 == 1){VertexColor = vec3(0.0, 1.0, 0.0);}
		else{VertexColor = vec3(1.0, 0.0, 0.0);}
	}
}