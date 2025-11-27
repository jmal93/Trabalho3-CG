#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <string>

struct VertexData {
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
};

class ImportedModel {
public:
    ImportedModel(const char* path);
    ~ImportedModel();

    void Draw() const;

private:
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;

    GLsizei indexCount = 0;

    void loadOBJ(const char* path);
};
