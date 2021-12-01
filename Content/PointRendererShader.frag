#version 460
out vec4 Fragment_Color;

struct DirectionalLight{
	vec3 DIRECTION;
	vec3 COLOR;
};
layout (std430) buffer LIGHTs{
	DirectionalLight DIRECTIONALLIGHT;
};
in vec4 PointColor;

uniform uint isPhongShadingActive;
void main(){
	if(isPhongShadingActive == 1){
		vec3 Normal = (PointColor.xyz * 2) - 1;
		Fragment_Color = vec4(vec3(dot(normalize(Normal), normalize(DIRECTIONALLIGHT.DIRECTION))), 1.0f);
	}
	else{Fragment_Color = vec4(PointColor);}
}