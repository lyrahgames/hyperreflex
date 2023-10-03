#include <hyperreflex/application.hpp>
//
#include <chaiscript/chaiscript.hpp>

using namespace chaiscript;

// namespace {

// }  // namespace

namespace hyperreflex {

struct object {
  string name;
  string description;
  Boxed_Value data;
};

struct application::impl {
  ChaiScript chai{};
  vector<object> objects{};
};

application::application() : viewer{10, 10, 780, 430} {
  init_imgui();
  init_event_handlers();
  init_chaiscript();
  this_app = this;
}

application::~application() noexcept {
  free_imgui();
  this_app = nullptr;
}

void application::init_chaiscript() {
  pimpl = make_unique<impl>();

  pimpl->objects.emplace_back(
      "load_surface", "Load a surface mesh from file.",
      var(fun([this](const string& path) { viewer.load_surface(path); })));

  pimpl->objects.emplace_back(
      "load_shader", "Load a shader from file.",
      var(fun([this](const string& path, const string& name) {
        viewer.load_shader(path, name);
      })));

  pimpl->objects.emplace_back("fit_view", "Reset to default view.",
                              var(fun([this] { viewer.fit_view(); })));

  pimpl->objects.emplace_back(
      "help", "Print available functions.", var(fun([this] {
        cout << "Available Functions:\n";
        for (auto& x : pimpl->objects)
          cout << '\t' << x.name << '\n' << "\t\t" << x.description << "\n\n";
        cout << flush;
      })));

  for (auto& x : pimpl->objects) pimpl->chai.add(x.data, x.name);
}

void application::eval_chaiscript(const filesystem::path& script) {
  try {
    pimpl->chai.eval_file(script);
  } catch (chaiscript::exception::eval_error& e) {
    cout << e.what() << endl;
  }
}

void application::eval_chaiscript(const string& code) {
  try {
    pimpl->chai.eval(code);
  } catch (chaiscript::exception::eval_error& e) {
    cout << e.what() << endl;
  }
}

}  // namespace hyperreflex
