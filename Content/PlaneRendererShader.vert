#version 460
uniform vec3 Vertex0;
uniform vec3 Vertex1;
uniform vec3 Vertex2;
uniform vec3 Vertex3;
uniform vec3 Vertex4;
uniform vec3 Vertex5;

layout (std430, binding = 0) buffer VIEW_MATRICES{
	mat4 PROJ_MATRIX;
	mat4 VIEW_MATRIX;
};


void main(){
	vec3 VertPos = vec3(0.0f);
	switch(gl_VertexID){
		case 0:
		VertPos = Vertex0;
		break;
		case 1:
		VertPos = Vertex1;
		break;
		case 2:
		VertPos = Vertex2;
		break;
		case 3:
		VertPos = Vertex3;
		break;
		case 4:
		VertPos = Vertex4;
		break;
		case 5:
		VertPos = Vertex5;
		break;
		default:
		VertPos = vec3(1.0f/0.0f);
		break;
	}
	gl_Position = PROJ_MATRIX * VIEW_MATRIX * vec4(VertPos, 1.0f);
}