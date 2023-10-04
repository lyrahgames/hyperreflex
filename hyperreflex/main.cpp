// #include <hyperreflex/viewer.hpp>
//
#include <hyperreflex/application.hpp>

using namespace std;

int main(int argc, const char* argv[]) {
  // if (argc != 2) {
  //   std::cout << "Usage:\n" << argv[0] << " <STL object file path>\n";
  //   return 0;
  // }

  const auto path = filesystem::path(argv[0]).parent_path();

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

  hyperreflex::application app{argc, argv};

  // app.viewer.load_surface(argv[1]);
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
