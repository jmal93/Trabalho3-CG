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

    m_model_ptr->Draw(); 
}