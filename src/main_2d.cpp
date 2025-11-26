
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "scene.h"
#include "state.h"
#include "camera2d.h"
#include "color.h"
#include "transform.h"
#include "error.h"
#include "shader.h"
#include "quad.h"
#include "triangle.h"
#include "disk.h"

#include <initializer_list>
#include <iostream>
#include <ostream>

static ScenePtr scene;
static CameraPtr camera;

class Orbit;
using OrbitPtr = std::shared_ptr<Orbit>;
class Orbit : public Engine 
{
  TransformPtr m_trf;
  float m_speed;
protected:
  Orbit (TransformPtr trf, float speed) 
  : m_trf(trf), m_speed(speed) 
  {
  }
public:
  static OrbitPtr Make (TransformPtr trf, float speed)
  {
    return OrbitPtr(new Orbit(trf, speed));
  }
  virtual void Update (float dt)
  {
    m_trf->Rotate(m_speed * (-dt)/30.0f*180.0f,0,0,1);
  }
};

static void initialize (void)
{
  // set background color: white 
  glClearColor(0.0f,0.0f,0.0f,1.0f);
  // enable depth test 
  glEnable(GL_DEPTH_TEST);

  // create objects
  camera = Camera2D::Make(0,10,0,10);

  auto trf_moon_orbit_disk = Transform::Make();
  trf_moon_orbit_disk->Translate(0.0f, 0.0f, 0.0f);

  auto trf_moon = Transform::Make();
  trf_moon->Translate(2.5f, 0.0f, 0.0f);
  trf_moon->Scale(0.5f, 0.5f, 1.0f);

  auto moon_orbit_disk = Node::Make(
    trf_moon_orbit_disk, {Node::Make(trf_moon, {Color::Make(190, 190, 190)}, {Disk::Make()})}
  );

  auto trf_earth = Transform::Make();
  trf_earth->Translate(4.5f, 0.0f, 1.0f);
  trf_earth->Scale(0.4f, 0.4f, 1.0f);
  auto earth = Node::Make(trf_earth, {Color::Make(0, 0, 1)}, {Disk::Make()});
  earth->AddNode(moon_orbit_disk);

  auto trf_earth_orbit_disk = Transform::Make();
  trf_earth_orbit_disk->Translate(5.0f, 5.0f, 0.0f);
  auto earth_orbit_disk = Node::Make(trf_earth_orbit_disk, {earth});

  auto trf_sun = Transform::Make();
  trf_sun->Translate(5.0f,5.0f,0.5f);
  trf_sun->Scale(2.0f,2.0f,1.0f);
  auto sun = Node::Make(trf_sun,{Color::Make(1,1,0)},{Disk::Make()});

  auto shader = Shader::Make();
  shader->AttachVertexShader("../shaders/2d/vertex.glsl");
  shader->AttachFragmentShader("../shaders/2d/fragment.glsl");
  shader->Link();

  auto root =  Node::Make(shader, {sun, earth_orbit_disk});
  scene = Scene::Make(root);
  scene->AddEngine(Orbit::Make(trf_earth_orbit_disk, 1.0f));
  scene->AddEngine(Orbit::Make(trf_moon_orbit_disk, 13.37f));
}

static void display (GLFWwindow* win)
{ 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear window 
  Error::Check("before render");
  scene->Render(camera);
  Error::Check("after render");
}

static void error (int code, const char* msg)
{
  printf("GLFW error %d: %s\n", code, msg);
  glfwTerminate();
  exit(0);
}

static void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void resize (GLFWwindow* win, int width, int height)
{
  glViewport(0,0,width,height);
}

static void update (float dt)
{
  scene->Update(dt);
}

int main ()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  glfwSetErrorCallback(error);

  GLFWwindow* win = glfwCreateWindow(600, 400, "Window title", nullptr, nullptr);
  assert(win);
  glfwSetFramebufferSizeCallback(win, resize);  // resize callback
  glfwSetKeyCallback(win, keyboard);            // keyboard callback

  glfwMakeContextCurrent(win);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }
  printf("OpenGL version: %s\n", glGetString(GL_VERSION));

  initialize();

  float t0 = float(glfwGetTime());
  while(!glfwWindowShouldClose(win)) {
    float t = float(glfwGetTime());
    update(t-t0);
    t0 = t;
    display(win);
    glfwSwapBuffers(win);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}

