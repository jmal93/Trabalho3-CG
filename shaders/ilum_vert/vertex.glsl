#version 410

layout(location = 0) in vec4 coord;
layout(location = 1) in vec3 normal;

uniform mat4 Mv; 
uniform mat4 Mn; 
uniform mat4 Mvp;

// Saídas para o Geometry Shader
out VertexData {
    vec3 position;    // Posição original
    vec3 normal;      // Normal original
} v_out;

void main(void) 
{
    // Passa dados brutos para o GS
    v_out.position = vec3(coord);
    v_out.normal = normal;
    
    // A transformação final será feita no GS
    gl_Position = Mvp * coord;
}