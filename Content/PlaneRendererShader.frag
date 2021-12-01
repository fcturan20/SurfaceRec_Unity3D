#version 460
out vec4 Fragment_Color;

uniform vec3 PlaneColor;

void main(){
	Fragment_Color = vec4(PlaneColor, 1.0f);
}