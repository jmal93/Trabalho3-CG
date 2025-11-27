#include "obj_loader.h" 

#include <glad/glad.h>   // Para funções OpenGL
#include <iostream>    // Para std::cerr
#include <fstream>
#include <sstream>
#include <map>
#include <tuple>
#include <cstring> 
#include <vector>
#include <algorithm> // Para std::replace

// Implementação do Carregamento (Agora suporta Quads via Triangulação Fan)
void  ImportedModel::loadOBJ(const char* path) {
    
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    std::vector<VertexData> final_vertices;
    std::vector<unsigned int> final_indices;
    std::map<std::tuple<unsigned int, unsigned int, unsigned int>, unsigned int> vertex_map;

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERRO: Nao foi possivel abrir o arquivo OBJ: " << path << std::endl;
        return; 
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string lineHeader;
        ss >> lineHeader;

        if (lineHeader == "v") {
            glm::vec3 position;
            if (ss >> position.x >> position.y >> position.z)
                temp_vertices.push_back(position);
        } else if (lineHeader == "vt") {
            glm::vec2 uv;
            if (ss >> uv.x >> uv.y) {
                uv.y = -uv.y; 
                temp_uvs.push_back(uv);
            }
        } else if (lineHeader == "vn") {
            glm::vec3 normal;
            if (ss >> normal.x >> normal.y >> normal.z)
                temp_normals.push_back(normal);
        } else if (lineHeader == "f") {
            
            // --- LOGICA ROBUSTA PARA PARSING DE FACES (SUPORTA QUADS/NGONS) ---
            std::vector<unsigned int> face_v_indices, face_vt_indices, face_vn_indices;
            std::string segment;

            // Le os vertices da face, um de cada vez (ex: "1/1/1", "2/2/2", etc.)
            while (ss >> segment) {
                // Substitui barras '/' por espacos para facilitar a leitura com stringstream
                std::replace(segment.begin(), segment.end(), '/', ' ');
                std::stringstream segment_ss(segment);
                
                unsigned int v, vt, vn;
                // Le os 3 indices (v, vt, vn)
                if (segment_ss >> v >> vt >> vn) {
                    face_v_indices.push_back(v);
                    face_vt_indices.push_back(vt);
                    face_vn_indices.push_back(vn);
                }
            }

            // --- TRIANGULACAO E REINDEXACAO ---
            if (face_v_indices.size() < 3) {
                 // Nao deve acontecer se o parsing for bem feito, mas eh uma seguranca
                 continue;
            }

            // Triangulacao (Fan Triangulation): Cria triangulos a partir do primeiro vertice (0)
            for (size_t i = 0; i < face_v_indices.size() - 2; ++i) {
                // Os vertices de cada triangulo sao: (0, i+1, i+2)
                
                // Vetor com os 3 conjuntos de indices do novo triangulo
                std::vector<std::tuple<unsigned int, unsigned int, unsigned int>> triangle_keys;
                
                // Vértice A (sempre o primeiro da face)
                triangle_keys.push_back({face_v_indices[0], face_vt_indices[0], face_vn_indices[0]});
                // Vértice B
                triangle_keys.push_back({face_v_indices[i+1], face_vt_indices[i+1], face_vn_indices[i+1]});
                // Vértice C
                triangle_keys.push_back({face_v_indices[i+2], face_vt_indices[i+2], face_vn_indices[i+2]});

                // Reindexacao dos 3 vertices do novo triangulo
                for (const auto& key : triangle_keys) {
                    unsigned int idx_v = std::get<0>(key);
                    unsigned int idx_uv = std::get<1>(key);
                    unsigned int idx_n = std::get<2>(key);
                    
                    if (vertex_map.count(key)) {
                        final_indices.push_back(vertex_map[key]);
                    } else {
                        // Novo vértice. Indices OBJ comecam em 1
                        VertexData newVertex = {
                            temp_vertices[idx_v - 1], 
                            temp_uvs[idx_uv - 1], 
                            temp_normals[idx_n - 1]
                        };
                        
                        final_vertices.push_back(newVertex);
                        unsigned int newIndex = final_vertices.size() - 1;
                        final_indices.push_back(newIndex);
                        vertex_map[key] = newIndex;
                    }
                }
            }
            // --- FIM DA TRIANGULACAO E REINDEXACAO ---
        }
    }
    
    // Nao eh necessario verificar se o arquivo esta aberto, pois o while(getline) cuida disso.
    // Garante que o arquivo sera fechado
    // file.close(); // file eh fechado automaticamente por ser std::ifstream

    // 4. CONFIGURAÇÃO DO OPENGL (AGORA USANDO OS MEMBROS DA CLASSE)
    
    if (final_indices.empty() || final_vertices.empty()) {
        std::cerr << "AVISO: Modelo carregado, mas sem vertices ou indices validos. Nao foi possivel desenhar." << std::endl;
        // VAO/VBO/EBO permanecerao 0, o que e seguro, mas o modelo nao sera desenhado.
        return;
    }
    
    indexCount = final_indices.size();

    // 2. Gera os Buffers e Arrays usando os membros VAO, VBO, EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // VBO (dados dos vertices)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, final_vertices.size() * sizeof(VertexData), final_vertices.data(), GL_STATIC_DRAW);

    // EBO (indices)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); // Correcao: Use GL_ELEMENT_ARRAY_BUFFER duas vezes
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, final_indices.size() * sizeof(unsigned int), final_indices.data(), GL_STATIC_DRAW);

    // Configuração dos atributos do vértice (Layouts)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, uv));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, normal));

    glBindVertexArray(0);   
}