#version 410

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

// Fator de encolhimento (ajuste conforme necessário)
const float shrink_factor = 0.7;

// Uniforms (os mesmos do seu VS original)
uniform mat4 Mv; 
uniform mat4 Mn; 
uniform mat4 Mvp;

uniform vec4 lpos;
uniform vec4 lamb;
uniform vec4 ldif;
uniform vec4 lspe;

uniform vec4 mamb;
uniform vec4 mdif;
uniform vec4 mspe;
uniform float mshi;
uniform float mopacity;

// Entradas do Vertex Shader
in VertexData {
    vec3 position;
    vec3 normal;
} v_in[];

// Saída para o Fragment Shader
out vec4 color;

void main(void)
{
    // Calcula o centro de gravidade do triângulo
    vec3 center = vec3(0.0);
    for(int i = 0; i < 3; i++) {
        center += v_in[i].position;
    }
    center /= 3.0;

    // Para cada vértice do triângulo original
    for(int i = 0; i < 3; i++) {
        // Aplica o encolhimento em direção ao centro
        vec3 shrunk_pos = center + shrink_factor * (v_in[i].position - center);
        
        // CÁLCULO DA ILUMINAÇÃO (igual ao seu VS original)
        vec3 veye = vec3(Mv * vec4(shrunk_pos, 1.0));
        vec3 light;
        if(lpos.w == 0) 
            light = normalize(vec3(lpos));
        else 
            light = normalize(vec3(lpos) - veye); 
        
        vec3 neye = normalize(vec3(Mn * vec4(v_in[i].normal, 0.0)));
        float ndotl = dot(neye, light);
        
        color = mamb * lamb + mdif * ldif * max(0.0, ndotl); 
        if(ndotl > 0) {
            vec3 refl = normalize(reflect(-light, neye));
            color += mspe * lspe * pow(max(0.0, dot(refl, normalize(-veye))), mshi); 
        }
        color.a = mopacity;
        
        // Posição final transformada
        gl_Position = Mvp * vec4(shrunk_pos, 1.0);
        
        EmitVertex();
    }
    EndPrimitive();
}