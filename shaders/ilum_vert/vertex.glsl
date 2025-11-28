#version 410

layout(location = 0) in vec4 coord;
layout(location = 1) in vec3 normal;

uniform mat4 Mv; 
uniform mat4 Mn; 
uniform mat4 Mvp;

out VertexData {
    vec3 position;
    vec3 normal;
} v_out;

void main(void) 
{
    v_out.position = vec3(coord);
    v_out.normal = normal;
    
    gl_Position = Mvp * coord;
}