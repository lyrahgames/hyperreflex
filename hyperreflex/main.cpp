// #include <hyperreflex/viewer.hpp>
//
#include <hyperreflex/application.hpp>
//
#include <CLI/CLI.hpp>

using namespace std;
using namespace spdlog;

int main(int argc, const char* argv[]) {
  const auto path = filesystem::path(argv[0]).parent_path();

  CLI::App cli{
      "hyperreflex: Smoothing of Surface Mesh Curves by Using Geodesics and "
      "Penalty Potentials"};

  string log_pattern = "%^[%l]%$[tid %t]\n%v\n";
  cli.add_option("--log-pattern", log_pattern, "Set log pattern.");

  filesystem::path mesh_path{};
  cli.add_option("-m,--mesh,mesh-pos", mesh_path,
                 "Surface mesh file to be loaded.");

  filesystem::path chai_script{};
  cli.add_option("--chai", chai_script,
                 "Load and execute given ChaiScript file at start-up.");

  try {
    cli.parse(argc, argv);
  } catch (const CLI::ParseError& e) {
    stringstream out{}, err{};
    const auto exit_code = cli.exit(e, out, err);
    if (!err.view().empty()) error("Program Options:\n{}", err.view());
    if (!out.view().empty()) info("Program Options:\n{}", out.view());
    return exit_code;
  }

  spdlog::set_pattern(log_pattern);

  hyperreflex::application app{};

  if (!chai_script.empty()) app.eval_chaiscript(chai_script);
  if (!mesh_path.empty()) app.viewer.async_load_surface(mesh_path);

  app.viewer.load_shader(path / "shader/default", "default");
  app.viewer.load_shader(path / "shader/heat", "flat");
  app.viewer.load_shader(path / "shader/points", "points");
  app.viewer.load_shader(path / "shader/initial", "initial");
  app.viewer.load_shader(path / "shader/critical", "critical");
  app.viewer.load_shader(path / "shader/contours", "contours");
  app.viewer.load_shader(path / "shader/selection", "selection");
  app.viewer.load_shader(path / "shader/boundary", "boundary");
  app.viewer.load_shader(path / "shader/unoriented", "unoriented");
  app.viewer.load_shader(path / "shader/inconsistent", "inconsistent");
  app.viewer.load_shader(path / "shader/line", "line");

  app.run();
}
