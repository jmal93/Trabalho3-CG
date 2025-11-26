#include "obj_loader.h"

#include <fstream>
#include <sstream>
#include <map>
#include <tuple>
#include <cstring>

void  ImportedModel::loadOBJ(const char* path) {
    
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    FILE* file = fopen(path, "r");
    if (file == NULL) {
        std::cerr << "ERRO: Impossivel abrir o arquivo! Certifique-se de que o caminho esta correto. -> " << path << std::endl;
        return;
    }

    while (1) {
        char lineHeader[128];
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break;

        if (strcmp(lineHeader, "v") == 0) {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);

        } else if (strcmp(lineHeader, "vt") == 0) {
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y);
            uv.y = -uv.y; 
            temp_uvs.push_back(uv);

        } else if (strcmp(lineHeader, "vn") == 0) {
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);

        } else if (strcmp(lineHeader, "f") == 0) {
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", 
                &vertexIndex[0], &uvIndex[0], &normalIndex[0], 
                &vertexIndex[1], &uvIndex[1], &normalIndex[1], 
                &vertexIndex[2], &uvIndex[2], &normalIndex[2]
            );

            if (matches != 9) {
                std::cerr << "ERRO: O carregador OBJ aceita apenas triangulos (f v/vt/vn v/vt/vn v/vt/vn)!" << std::endl;
                fclose(file);
                return;
            }

            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices.push_back(uvIndex[0]);
            uvIndices.push_back(uvIndex[1]);
            uvIndices.push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        }
    }
    
    fclose(file);

    std::vector<VertexData> final_vertices;
    std::vector<unsigned int> final_indices;
    std::map<std::tuple<unsigned int, unsigned int, unsigned int>, unsigned int> vertex_map;

    for (size_t i = 0; i < vertexIndices.size(); i++) {
        std::tuple<unsigned int, unsigned int, unsigned int> index_key = {
            vertexIndices[i], 
            uvIndices[i], 
            normalIndices[i]
        };

        if (vertex_map.count(index_key)) {
            final_indices.push_back(vertex_map[index_key]);
        } else {
            VertexData newVertex = {
                temp_vertices[vertexIndices[i] - 1], 
                temp_uvs[uvIndices[i] - 1], 
                temp_normals[normalIndices[i] - 1]
            };
            
            final_vertices.push_back(newVertex);
            unsigned int newIndex = final_vertices.size() - 1;
            final_indices.push_back(newIndex);
            
            vertex_map[index_key] = newIndex;
        }
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
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
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