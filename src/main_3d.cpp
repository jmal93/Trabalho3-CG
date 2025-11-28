
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "arcball.h"
#include "scene.h"
#include "state.h"
#include "camera3d.h"
#include "material.h"
#include "texture.h"
#include "transform.h"
#include "cube.h"
#include "quad.h"
#include "sphere.h"
#include "error.h"
#include "shader.h"
#include "light.h"
#include "light.h"
#include "polyoffset.h"

#include "obj_loader.h"
#include "model_shape.h"

#include <iostream>
#include <cassert>

static float viewer_pos[3] = {2.0f, 3.5f, 4.0f};

static ScenePtr scene;
static ScenePtr reflector;
static Camera3DPtr camera;
static ArcballPtr arcball;

ImportedModel* modeloTeste = nullptr; 

static void initialize(void)
{
  // set background color: white
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  // enable depth test
  glEnable(GL_DEPTH_TEST);
  //glEnable(GL_CULL_FACE); // cull back faces

  // create objects
  camera = Camera3D::Make(viewer_pos[0], viewer_pos[1], viewer_pos[2]);
  // camera->SetOrtho(true);
  arcball = camera->CreateArcball();

  // TESTE DE MODELO =======================================
  modeloTeste = new ImportedModel("./models/planta.obj"); 

  ShapePtr object_shape = ModelShape::Make(modeloTeste); 
  TransformPtr trf_object = Transform::Make();
  trf_object->Scale(0.05f, 0.05f, 0.05f); 
  trf_object->Translate(1.0f, 0.0f, -1.0f); 
  AppearancePtr object_appearance = Material::Make(1.0f, 0.7f, 0.7f);
  NodePtr object_node = Node::Make(trf_object, {object_appearance}, {object_shape});
  //FIM DO TESTE ===========================================


  // LightPtr light = ObjLight::Make(viewer_pos[0],viewer_pos[1],viewer_pos[2]);
  LightPtr light = Light::Make(0.0f, 0.0f, 0.0f, 1.0f, "camera");

  AppearancePtr white = Material::Make(1.0f, 1.0f, 1.0f);
  AppearancePtr floor_appearance = Material::Make(1.0f, 0.0f, 0.0f, 0.5f);

  TransformPtr floor_transform = Transform::Make();
  floor_transform->Scale(3.0f, 3.0f, 3.0f);
  floor_transform->Rotate(-90, 1.0f, 0.0f, 0.0f);
  floor_transform->Translate(-0.5f, -0.5f, 0.0f);
  TransformPtr sphere_transform = Transform::Make();
  sphere_transform->Scale(0.5f, 0.5f, 0.5f);
  sphere_transform->Translate(0.0f, 1.0f, 0.0f);

  Error::Check("before shps");
  ShapePtr cube = Cube::Make();
  ShapePtr quad = Quad::Make();
  ShapePtr sphere = Sphere::Make();
  Error::Check("after shps");

  // create shader
  ShaderPtr shader = Shader::Make(light, "world");
  shader->AttachVertexShader("shaders/ilum_vert/vertex.glsl");
  shader->AttachFragmentShader("shaders/ilum_vert/fragment.glsl");
  shader->AttachGeometryShader("shaders/ilum_vert/geometry.glsl");
  shader->Link();

  // Define a different shader for texture mapping
  // An alternative would be to use only this shader with a "white" texture for untextured objects
  ShaderPtr shd_tex = Shader::Make(light, "world");
  shd_tex->AttachVertexShader("shaders/ilum_vert/vertex_texture.glsl");
  shd_tex->AttachFragmentShader("shaders/ilum_vert/fragment_texture.glsl");
  shd_tex->Link();

  NodePtr sphere_node = Node::Make(sphere_transform, {white}, {sphere});
  NodePtr root = Node::Make(shader, {object_node});
  scene = Scene::Make(root);

  NodePtr floor_node = Node::Make(floor_transform, {floor_appearance}, {quad});
  reflector = Scene::Make(Node::Make(shader, {floor_node}));
}

static void display(GLFWwindow *win)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // clear window
  Error::Check("before render");

  // desenha refletor no stencil
  glEnable(GL_STENCIL_TEST);
  glStencilFunc(GL_NEVER, 1, 0xFFFF);
  glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
  reflector->Render(camera);

  // desenha cena refletida
  glStencilFunc(GL_EQUAL, 1, 0xFFFF);
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
  NodePtr root = scene->GetRoot();
  TransformPtr trf = Transform::Make();
  trf->Scale(1.0f, -1.0f, 1.0f);
  root->SetTransform(trf);
  glFrontFace(GL_CW);
  scene->Render(camera);
  glFrontFace(GL_CCW);
  root->SetTransform(nullptr);
  glDisable(GL_STENCIL_TEST);

  // desenha cena
  scene->Render(camera);

  // desenha refletor
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  reflector->Render(camera);
  glDisable(GL_BLEND);
  Error::Check("after render");
}

static void error(int code, const char *msg)
{
  printf("GLFW error %d: %s\n", code, msg);
  glfwTerminate();
  exit(0);
}

static void keyboard(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void resize(GLFWwindow *win, int width, int height)
{
  glViewport(0, 0, width, height);
}

static void cursorpos(GLFWwindow *win, double x, double y)
{
  // convert screen pos (upside down) to framebuffer pos (e.g., retina displays)
  int wn_w, wn_h, fb_w, fb_h;
  glfwGetWindowSize(win, &wn_w, &wn_h);
  glfwGetFramebufferSize(win, &fb_w, &fb_h);
  x = x * fb_w / wn_w;
  y = (wn_h - y) * fb_h / wn_h;
  arcball->AccumulateMouseMotion(int(x), int(y));
}
static void cursorinit(GLFWwindow *win, double x, double y)
{
  // convert screen pos (upside down) to framebuffer pos (e.g., retina displays)
  int wn_w, wn_h, fb_w, fb_h;
  glfwGetWindowSize(win, &wn_w, &wn_h);
  glfwGetFramebufferSize(win, &fb_w, &fb_h);
  x = x * fb_w / wn_w;
  y = (wn_h - y) * fb_h / wn_h;
  arcball->InitMouseMotion(int(x), int(y));
  glfwSetCursorPosCallback(win, cursorpos); // cursor position callback
}
static void mousebutton(GLFWwindow *win, int button, int action, int mods)
{
  if (action == GLFW_PRESS)
  {
    glfwSetCursorPosCallback(win, cursorinit); // cursor position callback
  }
  else                                      // GLFW_RELEASE
    glfwSetCursorPosCallback(win, nullptr); // callback disabled
}

int main()
{
  if (!glfwInit())
  {
    std::cout << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);      // required for mac os
  glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE); // option for mac os
#endif

  glfwSetErrorCallback(error);

  GLFWwindow *win = glfwCreateWindow(600, 400, "Window title", nullptr, nullptr);
  assert(win);
  glfwSetFramebufferSizeCallback(win, resize);  // resize callback
  glfwSetKeyCallback(win, keyboard);            // keyboard callback
  glfwSetMouseButtonCallback(win, mousebutton); // mouse button callback

  glfwMakeContextCurrent(win);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }
  printf("OpenGL version: %s\n", glGetString(GL_VERSION));

  initialize();

  while (!glfwWindowShouldClose(win))
  {
    display(win);
    glfwSwapBuffers(win);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}
