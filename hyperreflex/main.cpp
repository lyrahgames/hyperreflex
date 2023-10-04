// #include <hyperreflex/viewer.hpp>
//
#include <hyperreflex/application.hpp>
//
#include <CLI/CLI.hpp>

using namespace std;
using namespace spdlog;

int main(int argc, const char* argv[]) {
  spdlog::set_pattern("%^[%l]%$[tid %t]\n%v\n");

  const auto path = filesystem::path(argv[0]).parent_path();

  CLI::App cli{
      "hyperreflex: Smoothing of Surface Mesh Curves by Using Geodesics and "
      "Penalty Potentials"};

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

  // if (argc != 2) {
  //   std::cout << "Usage:\n" << argv[0] << " <STL object file path>\n";
  //   return 0;
  // }

  // hyperreflex::viewer viewer{};
  // viewer.load_surface(argv[1]);

  // viewer.load_shader(path / "shader/default", "default");
  // viewer.load_shader(path / "shader/heat", "flat");
  // viewer.load_shader(path / "shader/points", "points");
  // viewer.load_shader(path / "shader/initial", "initial");
  // viewer.load_shader(path / "shader/critical", "critical");
  // viewer.load_shader(path / "shader/contours", "contours");
  // viewer.load_shader(path / "shader/selection", "selection");
  // viewer.load_shader(path / "shader/boundary", "boundary");
  // viewer.load_shader(path / "shader/unoriented", "unoriented");
  // viewer.load_shader(path / "shader/inconsistent", "inconsistent");

  // // viewer.load_surface_shader(path / "shader/default");
  // // viewer.load_selection_shader(path / "shader/selection");
  // // viewer.load_surface_curve_point_shader((path /
  // "shader/points").c_str()); viewer.run();

  // hyperreflex::application::init();
  // hyperreflex::application::run();

  hyperreflex::application app{};

  if (!mesh_path.empty()) app.viewer.async_load_surface(mesh_path);
  if (!chai_script.empty()) app.eval_chaiscript(chai_script);

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

  app.run();
}
