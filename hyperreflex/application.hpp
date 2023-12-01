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
  application_context(int width = 512, int height = 512);
  virtual ~application_context() noexcept;

 protected:
  GLFWwindow* window = nullptr;
};

class application final : public application_context {
 public:
  // using base = application_context;
  // using base::error;
  // using base::info;

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

  void init_chaiscript();

 public:
  void eval_chaiscript(const filesystem::path& script);
  void eval_chaiscript(const string& code);

 public:
  const char* glsl_version = "#version 330";
  struct viewer viewer;

  vec2 old_mouse_pos;
  vec2 mouse_pos;

  bool command_prompt = false;
  string command_buffer;

  struct impl;
  unique_ptr<impl> pimpl{};

  filesystem::path bin_path{};

  static constexpr int init_width = 512;
  static constexpr int init_height = 512;
};

// GLFW works only with a global state.
//
extern application* this_app;

}  // namespace hyperreflex
