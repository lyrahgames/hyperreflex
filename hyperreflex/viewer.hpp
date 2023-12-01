#pragma once
#include <hyperreflex/camera.hpp>
#include <hyperreflex/opengl/opengl.hpp>
#include <hyperreflex/points.hpp>
#include <hyperreflex/polyhedral_surface.hpp>
#include <hyperreflex/shader_manager.hpp>
#include <hyperreflex/utility.hpp>
//
#include <geometrycentral/surface/edge_length_geometry.h>
#include <geometrycentral/surface/manifold_surface_mesh.h>
#include <geometrycentral/surface/vertex_position_geometry.h>
//
#include <igl/heat_geodesics.h>

namespace hyperreflex {

class viewer {
 public:
  viewer(int x, int y, int width, int height);

  void resize(int x, int y, int width, int height);
  void update();
  void update_view();
  void render();

  void set_view_should_update() noexcept;

  void turn(const vec2& angle);
  void shift(const vec2& pixels);
  void zoom(float scale);
  void look_at(float x, float y);

  void set_z_as_up();
  void set_y_as_up();

  void load_surface(const filesystem::path& path);
  void async_load_surface(const filesystem::path& path);
  void handle_surface_load_task();

  void fit_view();
  void print_surface_info();

  void load_shader(const filesystem::path& path, const string& name);

  void sort_surface_faces_by_depth();

  auto select_vertex(float x, float y) -> polyhedral_surface::vertex_id;
  void select_origin_vertex(float x, float y);
  void select_destination_vertex(float x, float y);

  void compute_topology_and_geometry();

  void remove_line_artifacts();

  void compute_dijkstra_path();
  void update_line();
  void shorten_line();

  void compute_heat_data();
  void update_heat();

  void add_normal_displacement();
  void remove_normal_displacement();

  void smooth_line();

  // private:
 public:
  bool running = false;
  bool view_should_update = false;

  // World Origin
  vec3 origin;
  // Basis Vectors of Right-Handed Coordinate System
  vec3 up{0, 1, 0};
  vec3 right{1, 0, 0};
  vec3 front{0, 0, 1};
  // Spherical/Horizontal Coordinates of Camera
  float radius = 10;
  float altitude = 0;
  float azimuth = 0;

  struct camera camera {};

  // polyhedral_surface surface{};
  scene surface{};

  bool surface_should_update = false;

  // The loading of mesh data can take quite a long time
  // and may let the window manager think the program is frozen
  // if the data would be loaded by a blocking call.
  // Here, an asynchronous task is used
  // to get rid of this unresponsiveness.
  future<void> surface_load_task{};
  float32 surface_load_time{};
  float32 surface_process_time{};
  //
  float bounding_radius;

  shader_manager shaders{};

  unique_ptr<geometrycentral::surface::ManifoldSurfaceMesh> mesh{};
  unique_ptr<geometrycentral::surface::VertexPositionGeometry> geometry{};

  // Drawing lines.
  //
  polyhedral_surface::vertex_id origin_vertex =  //
      polyhedral_surface::invalid;
  polyhedral_surface::vertex_id destination_vertex =  //
      polyhedral_surface::invalid;
  //
  bool selecting = false;
  points device_line;
  points device_initial_line;
  vector<polyhedral_surface::vertex_id> line_vids{};

  // Heat Geodsics from libigl
  //
  Eigen::MatrixXd surface_vertex_matrix;
  Eigen::MatrixXi surface_face_matrix;
  igl::HeatGeodesicsData<double> heat_data;
  Eigen::VectorXd heat;
  opengl::vertex_buffer device_heat{};
  vector<float> potential;
  //
  bool displacing = false;
  unique_ptr<geometrycentral::surface::VertexPositionGeometry>
      displaced_geometry{};

  unique_ptr<geometrycentral::surface::EdgeLengthGeometry> lifted_geometry{};

  float avg_edge_length = 1.0f;
  float tolerance = 1.0f;
  float bound = 1.0f;
  float transition = 1.0f;

  bool lighting = true;
  bool smooth_line_drawing = true;
  bool initial_line_drawing = true;
};

}  // namespace hyperreflex
