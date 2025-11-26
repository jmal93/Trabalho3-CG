#include "disk.h"

#include <cmath>
#include <glad/glad.h>
#include <vector>

DiskPtr Disk::Make(int nslice) {
  return DiskPtr(new Disk(nslice));
}

Disk::Disk(int nslice) {
  float radius = 1.0f;
  std::vector<float> coords;
  m_nslice = nslice;

  for (int i = 0; i < nslice; i++) {
    float angle = 2.0f * M_PI * i / nslice;
    float x = radius * cos(angle);
    float y = radius * sin(angle);

    coords.push_back(x);
    coords.push_back(y);
  }

  glGenVertexArrays(1, &m_vao);
  glBindVertexArray(m_vao);

  GLuint ids[1];
  glGenBuffers(1, ids);

  glBindBuffer(GL_ARRAY_BUFFER, ids[0]);
  glBufferData(GL_ARRAY_BUFFER, coords.size() * sizeof(float), coords.data(),
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);
}

Disk::~Disk() {}

void Disk::Draw(StatePtr) {
  glBindVertexArray(m_vao);
  glDrawArrays(GL_TRIANGLE_FAN, 0, m_nslice);
}
