#version 460
layout(location = 0) in vec3 Point_POS;
layout(location = 1) in vec4 Point_COLOR;

layout (std430, binding = 0) buffer VIEW_MATRICES{
	mat4 PROJ_MATRIX;
	mat4 VIEW_MATRIX;
};

out vec4 PointColor;

void main(){
	gl_Position = PROJ_MATRIX * VIEW_MATRIX * vec4(Point_POS, 1.0f);
	PointColor = Point_COLOR;
	if(PointColor.w == 0.0f){
	gl_Position = vec4(1.0/0.0);}
}