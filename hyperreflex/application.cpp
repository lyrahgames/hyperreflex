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
using namespace spdlog;
using namespace gl;
using namespace glm;

namespace hyperreflex {

application* this_app = nullptr;

application_context::application_context() {
  // Create GLFW handler for error messages.
  glfwSetErrorCallback([](int error_code, const char* description) {
    error("GLFW Error {}: {}", error_code, description);
  });

  // Initialize GLFW.
  glfwInit();

  // Set required OpenGL context version for the window.
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  // Force GLFW to use the core profile of OpenGL.
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 16);

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

void application::run() {
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    process_events();
    update();
    render();
    glfwSwapBuffers(window);
  }
}

void application::init_imgui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  // (void)io;
  // io.ConfigFlags |=
  //     ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  // io.ConfigFlags |=
  //     ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
  io.IniFilename = nullptr;

  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  command_buffer.resize(1024);
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

  if (command_prompt) {
    // ImGui::ShowDemoWindow();
    // ImGui::InputTextMultiline("command_prompt", nullptr, 0,
    //                           ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() *
    //                           16), ImGuiInputTextFlags_AllowTabInput);

    // ImGui::Begin("Hello, World!");
    // ImGui::Text("This is some useful text.");
    // ImGui::InputText("test", command_buffer.data(), command_buffer.size());
    // ImGui::End();

    ImGuiWindowFlags window_flags =
        // ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings
        //     // | ImGuiWindowFlags_NoFocusOnAppearing
        | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_Always);
    // ImVec2(1.0f, 1.0f));
    ImGui::SetNextWindowSize(ImVec2(520, 200));
    bool tmp = true;
    ImGui::Begin("Command Prompt", &tmp, window_flags);
    ImGui::SetItemDefaultFocus();
    ImGui::InputTextMultiline("log", command_buffer.data(),
                              command_buffer.size(),
                              ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 8),
                              ImGuiInputTextFlags_AllowTabInput);
    // ImGui::SetItemDefaultFocus();
    // ImGui::SetKeyboardFocusHere(-1);
    // ImGuiInputTextFlags input_text_flags =
    // ImGuiInputTextFlags_EnterReturnsTrue; ImGuiInputTextFlags_EscapeClearsAll
    // | ImGuiInputTextFlags_CallbackCompletion |
    // ImGuiInputTextFlags_CallbackHistory;
    // bool reclaim_focus = false;
    // if (ImGui::InputText("Input", command_buffer.data(),
    // command_buffer.size(),
    //                      input_text_flags)) {
    //   cout << command_buffer << endl;
    //   command_buffer.assign(command_buffer.size(), '\0');
    //   reclaim_focus = true;
    //   command_prompt = false;
    // }
    // if (reclaim_focus)
    ImGui::SetKeyboardFocusHere(-1);
    ImGui::End();
  }

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void application::init_event_handlers() {
  glfwSetFramebufferSizeCallback(
      window, [](GLFWwindow* window, int width, int height) {
        auto& app = *this_app;
        app.viewer.resize(10, 10, width - 20, height - 20);
      });

  glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode,
                                int action, int mods) {
    auto& app = *this_app;
    bool ctrl_pressed = mods & GLFW_MOD_CONTROL;

    // Bindings that need to be handled always.
    //
    if (ctrl_pressed && (action == GLFW_PRESS)) {
      switch (key) {
        case GLFW_KEY_SPACE:
          app.command_prompt = !app.command_prompt;
          return;
          break;

        case GLFW_KEY_ENTER:
          // cout << "commit" << endl;
          // cout << app.command_buffer << endl;
          app.eval_chaiscript(string(app.command_buffer.c_str()));
          app.command_buffer.assign(app.command_buffer.size(), '\0');
          app.command_prompt = false;
          return;
          break;

        default:
          break;
      }
    }

    // ImGui Key Handling
    //
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) {
      ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
      return;
    }

    // Bindings without modifier keys
    //
    if (action == GLFW_PRESS) {
      switch (key) {
        case GLFW_KEY_ESCAPE:
          glfwSetWindowShouldClose(window, GLFW_TRUE);
          break;
        case GLFW_KEY_SPACE:
          app.viewer.smooth_line();
          break;
        case GLFW_KEY_1:
          app.viewer.set_y_as_up();
          break;
        case GLFW_KEY_2:
          app.viewer.set_z_as_up();
          break;
        case GLFW_KEY_9:
          app.viewer.add_normal_displacement();
          break;
        case GLFW_KEY_0:
          app.viewer.remove_normal_displacement();
          break;

        case GLFW_KEY_UP:
          app.viewer.tolerance *= 1.1f;
          app.viewer.update_heat();
          app.viewer.smooth_line();
          break;
        case GLFW_KEY_DOWN:
          app.viewer.tolerance *= 0.9f;
          app.viewer.update_heat();
          app.viewer.smooth_line();
          break;

        case GLFW_KEY_H:
          app.viewer.lighting = !app.viewer.lighting;
          app.viewer.shaders.names["flat"]->second.shader.bind().set(
              "lighting", app.viewer.lighting);
          break;
        case GLFW_KEY_S:
          app.viewer.smooth_line_drawing = !app.viewer.smooth_line_drawing;
          break;
        case GLFW_KEY_Z:
          app.viewer.sort_surface_faces_by_depth();
          break;

        default:
          break;
      }
    }
  });

  glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button,
                                        int action, int mods) {
    ImGuiIO& io = ImGui::GetIO();
    // io.AddMouseButtonEvent(button, action == GLFW_PRESS);
    if (io.WantCaptureMouse) {
      ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
      return;
    }

    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
      this_app->viewer.look_at(this_app->mouse_pos.x, this_app->mouse_pos.y);

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
      this_app->viewer.select_origin_vertex(this_app->mouse_pos.x,
                                            this_app->mouse_pos.y);
  });

  glfwSetScrollCallback(window, [](GLFWwindow* window, double x, double y) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
      ImGui_ImplGlfw_ScrollCallback(window, x, y);
      return;
    }

    this_app->viewer.zoom(0.1 * y);
  });
}

void application::process_events() {
  // Compute the mouse move vector.
  old_mouse_pos = mouse_pos;
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);
  mouse_pos = vec2{xpos, ypos};
  const auto mouse_move = mouse_pos - old_mouse_pos;

  ImGuiIO& io = ImGui::GetIO();
  if (io.WantCaptureMouse) return;

  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
    const auto lshift_pressed =
        glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
    if (lshift_pressed)
      viewer.shift(mouse_move);
    else
      viewer.turn({-0.01f * mouse_move.x, 0.01f * mouse_move.y});
  }

  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
    if (mouse_move != vec2{})
      viewer.select_destination_vertex(mouse_pos.x, mouse_pos.y);
  }
}

void application::update() {
  viewer.update();
}

void application::render() {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);
  glClearColor(0, 0, 0, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  viewer.render();
  render_imgui();
}

}  // namespace hyperreflex
