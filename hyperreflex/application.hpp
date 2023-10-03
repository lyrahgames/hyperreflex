#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
//
#include <hyperreflex/utility.hpp>
#include <hyperreflex/viewer.hpp>

namespace hyperreflex {

// To be able to use OpenGL utilities
// in the state of the Viewer,
// an OpenGL context needs to be created first.
// We enforce this by inheriting from the context structure.
//
class application_context {
 public:
  application_context();
  virtual ~application_context() noexcept;

  void info(const auto& data) { cout << "INFO:\n" << data << endl; }
  void error(const auto& data) { cout << "ERROR:\n" << data << endl; }

 protected:
  GLFWwindow* window = nullptr;
};

class application final : public application_context {
 public:
  application();
  ~application() noexcept;

  void run();

 private:
  void init_imgui();
  void free_imgui() noexcept;
  void render_imgui();

  void init_event_handlers();
  void process_events();

  void update();
  void render();

 public:
  const char* glsl_version = "#version 330";
  struct viewer viewer;

  vec2 old_mouse_pos;
  vec2 mouse_pos;

  bool command_prompt = false;
  string command_buffer;
};

// GLFW works only with a global state.
//
extern application* this_app;

}  // namespace hyperreflex
