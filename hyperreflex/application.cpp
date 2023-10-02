#include <hyperreflex/application.hpp>
//
#include <iostream>
#include <stdexcept>
#include <string>
//
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
//
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
//

using namespace std;
using namespace gl;
using namespace glm;

namespace hyperreflex {

application* this_app = nullptr;

application_context::application_context() {
  // Create GLFW handler for error messages.
  //
  glfwSetErrorCallback([](int error, const char* description) {
    throw runtime_error("GLFW Error " + to_string(error) + ": " + description);
  });

  // Initialize GLFW.
  //
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
}

application_context::~application_context() noexcept {
  if (window) glfwDestroyWindow(window);
  glfwTerminate();
}

application::application() : viewer{10, 10, 780, 430} {
  init_window();
  init_imgui();
  this_app = this;
}

application::~application() noexcept {
  free_imgui();
  free_window();
  this_app = nullptr;
}

void application::init_window() {
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
  glfwSetFramebufferSizeCallback(
      window, [](GLFWwindow* window, int width, int height) {
        this_app->viewer.resize(10, 10, width - 20, height - 20);
      });

  glfwSetScrollCallback(window, [](GLFWwindow* window, double x, double y) {
    this_app->viewer.zoom(0.1 * y);
  });
}

void application::free_window() noexcept {}

void application::init_imgui() {
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
}

void application::free_imgui() noexcept {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void application::render_imgui() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGui::ShowDemoWindow();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void application::run() {
  // Start application loop.
  while (!glfwWindowShouldClose(window) /*&& viewer->running()*/) {
    // Handle user and OS events.
    glfwPollEvents();
    process_events();

    viewer.update();

    // glClear(GL_COLOR_BUFFER_BIT);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    viewer.render();
    render_imgui();

    glfwSwapBuffers(window);
  }
}

void application::process_events() {
  // Compute the mouse move vector.
  old_mouse_pos = mouse_pos;
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);
  mouse_pos = vec2{xpos, ypos};
  const auto mouse_move = mouse_pos - old_mouse_pos;

  // Left mouse button should rotate the camera by using spherical coordinates.
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    viewer.turn({-0.01f * mouse_move.x, 0.01f * mouse_move.y});

  // Right mouse button should translate the camera.
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    viewer.shift(mouse_move);
}

}  // namespace hyperreflex
