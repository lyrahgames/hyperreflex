#include <hyperreflex/application.hpp>
//
#include <iostream>
#include <stdexcept>
#include <string>
//
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
//
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
//
#include <glm/glm.hpp>
//
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

using namespace std;
using namespace gl;
using namespace glm;

namespace hyperreflex::application {

namespace {

// Window and OpenGL Context
GLFWwindow* window = nullptr;
bool is_initialized = false;
// ::viewer::viewer* viewer = nullptr;

const char* glsl_version = "#version 330";

// Mouse Interaction
vec2 old_mouse_pos;
vec2 mouse_pos;

// RAII Destructor Simulator
// To make sure that the application::free function
// can be viewed as a destructor and adheres to RAII principle,
// we use a global variable of a simple type without a state
// and a destructor calling the application::free function.
struct raii_destructor_t {
  ~raii_destructor_t() { free(); }
} raii_destructor{};

// Helper Function Declarations
// Create window with OpenGL context.
void init_window();

void process_events();

}  // namespace

void init(int argc, char** argv) {
  // Do not initialize if it has already been done.
  if (is_initialized) return;

  init_window();

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // if (!viewer) viewer = new ::viewer::viewer;
  // viewer->load_model(argv[1]);
  // viewer->load_shader(argv[2]);
  // viewer->start();

  // Update private state.
  is_initialized = true;
  cout << "Created OpenGL test application without errors!" << endl;
}

void free() {
  // An uninitialized application cannot be destroyed.
  if (!is_initialized) return;

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  // if (viewer) delete viewer;

  if (window) glfwDestroyWindow(window);
  glfwTerminate();

  // Update private state.
  is_initialized = false;
  cout << "Destroyed OpenGL test application without errors!" << endl;
}

void run() {
  // Make sure application::init has been called.
  if (!is_initialized) init();

  // Start application loop.
  while (!glfwWindowShouldClose(window) /*&& viewer->running()*/) {
    // Handle user and OS events.
    glfwPollEvents();
    process_events();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();

    // viewer->update();
    // viewer->render();

    ImGui::Render();

    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Swap buffers to display the
    // new content of the frame buffer.
    glfwSwapBuffers(window);
  }
}

// Private Member Function Implementations
namespace {

void init_window() {
  // Create GLFW handler for error messages.
  glfwSetErrorCallback([](int error, const char* description) {
    throw runtime_error("GLFW Error " + to_string(error) + ": " + description);
  });

  // Initialize GLFW.
  glfwInit();

  // Set required OpenGL context version for the window.
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  // Force GLFW to use the core profile of OpenGL.
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create the window to render in.
  window = glfwCreateWindow(800, 450, "hyperreflex", nullptr, nullptr);

  // Initialize the OpenGL context for the current window by using glbinding.
  glfwMakeContextCurrent(window);
  glbinding::initialize(glfwGetProcAddress);

  // Make window to be closed when pressing Escape
  // by adding key event handler.
  glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode,
                                int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
      // viewer->stop();
    }
  });

  // Add resize handler.
  glfwSetFramebufferSizeCallback(window,
                                 [](GLFWwindow* window, int width, int height) {
                                   // viewer->resize(width, height);
                                 });

  glfwSetScrollCallback(window, [](GLFWwindow* window, double x, double y) {
    // viewer->zoom(0.1 * y);
  });
}

void process_events() {
  // Compute the mouse move vector.
  old_mouse_pos = mouse_pos;
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);
  mouse_pos = vec2{xpos, ypos};
  const auto mouse_move = mouse_pos - old_mouse_pos;

  // Left mouse button should rotate the camera by using spherical coordinates.
  // if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
  //   viewer->turn(0.01f * mouse_move);

  // Right mouse button should translate the camera.
  // if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
  //   viewer->shift(mouse_move);
}

}  // namespace

}  // namespace hyperreflex::application
