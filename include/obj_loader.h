#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <iostream>

struct VertexData {
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
};

class ImportedModel {
public:
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;
    GLsizei indexCount = 0;

    ImportedModel(const char* path) {
        std::cout << "Carregando modelo: " << path << std::endl;
        loadOBJ(path);
    }
    
    ~ImportedModel() {
        if (VAO != 0) {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &EBO);
            std::cout << "Recursos da Mesh liberados da GPU." << std::endl;
        }
    }
    
    void Draw() const {
        if (VAO == 0) return;

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0); 
        glBindVertexArray(0);
    }

private:
    void loadOBJ(const char* path);
};
