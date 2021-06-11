#version 460
layout(points) in;
layout(line_strip, max_vertices = 2) out;

layout (std430, binding = 0) buffer VIEW_MATRICES{
	mat4 PROJ_MATRIX;
	mat4 VIEW_MATRIX;
};
#define MAX_WORLDOBJECTNUMBER 1000
layout (std430, binding = 1) buffer MODEL_MATRICES{
	mat4 MODEL_MATRICes[MAX_WORLDOBJECTNUMBER];
};

uniform float normal_length = 0.02f;
uniform vec3 color = vec3(1.0, 1.0, 1.0);
uniform uint ShowNormal;


in vec3 vertex_normal[];
out vec3 vertex_color;

uniform float max_render_dist = 1000.0f;

void main()
{
    mat4 mvp = MODEL_MATRICes[0];
    mat4 nm = transpose(inverse(mvp));
    vec3 normal = vertex_normal[0];
    normal = normalize(normal);

    vec3 RotatedNormal = vec4(nm * vec4(normal, 0.0)).xyz;
    vertex_color = (normalize(RotatedNormal) + 1.0f) / 2;

    
    if(ShowNormal != 0)
    {
        vec4 v0 = gl_in[0].gl_Position;
        gl_Position = PROJ_MATRIX * VIEW_MATRIX * MODEL_MATRICes[0] * v0;
        if(length(gl_Position) < max_render_dist){
            EmitVertex();

            vec4 v1 = v0 + vec4(normal * normal_length, 0.0);
            gl_Position = PROJ_MATRIX * VIEW_MATRIX * MODEL_MATRICes[0] * v1;
            EmitVertex();
        }
    } 
    else{
        vec4 v0 = gl_in[0].gl_Position;
        gl_Position = PROJ_MATRIX * VIEW_MATRIX * MODEL_MATRICes[0] * v0;
        vertex_color = vec3(1.0f);
        EmitVertex();
        
        mat4 inverse_mvp = inverse(PROJ_MATRIX * VIEW_MATRIX * MODEL_MATRICes[0]);
        vec4 v1 = v0 + (inverse_mvp * vec4(vec3(normal_length), 0.0f));
        gl_Position = PROJ_MATRIX * VIEW_MATRIX * MODEL_MATRICes[0] * v1;
        EmitVertex();
    }

    EndPrimitive();
}