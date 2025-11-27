#include "model_shape.h"
#include "shape.h"
#include "error.h"
#include "grid.h"

#include <iostream>

#include <glad/glad.h>

ShapePtr ModelShape::Make (ImportedModel* model)
{
  return std::make_shared<ModelShape>(model);
}

ModelShape::ModelShape (ImportedModel* model) : m_model_ptr(model)
{
  if (model == nullptr) {
        throw std::runtime_error("ModelShape::ModelShape: O ImportedModel pointer nao pode ser nulo.");
    }
    Error::Check("ModelShape criado com sucesso.");
}

ModelShape::~ModelShape () 
{
}

void ModelShape::Draw (StatePtr )
{
  // Debug: ver se o shader está realmente ligado
    GLint prog = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
    std::cout << "[ModelShape::Draw] current program = " << prog << std::endl;

    if (!m_model_ptr) return;

    m_model_ptr->Draw(); 
}