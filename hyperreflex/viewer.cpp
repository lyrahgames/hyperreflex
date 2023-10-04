#include <hyperreflex/viewer.hpp>
//
#include <hyperreflex/math.hpp>
//
#include <geometrycentral/surface/flip_geodesics.h>
#include <geometrycentral/surface/halfedge_element_types.h>
//
#include <igl/avg_edge_length.h>

using namespace spdlog;

namespace hyperreflex {

viewer::viewer(int x, int y, int width, int height) {
  // To initialize the viewport and matrices,
  // window has to be resized at least once.
  resize(x, y, width, height);

  // Setup for OpenGL
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH_TEST);
  // glEnable(GL_POINT_SMOOTH);
  // glEnable(GL_POINT_SPRITE);
  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glPointSize(10.0f);
  glLineWidth(4.0f);

  surface.setup();
  device_heat.bind();
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);

  device_initial_line.setup();
  device_line.setup();
}

void viewer::resize(int x, int y, int width, int height) {
  glViewport(x, y, width, height);
  camera.set_screen_viewport(x, y, width, height);
  view_should_update = true;
}

void viewer::update_view() {
  // Computer camera position by using spherical coordinates.
  // This transformation is a variation of the standard
  // called horizontal coordinates often used in astronomy.
  auto p = cos(altitude) * sin(azimuth) * right +  //
           cos(altitude) * cos(azimuth) * front +  //
           sin(altitude) * up;
  p *= radius;
  p += origin;
  camera.move(p).look_at(origin, up);

  camera.set_near_and_far(std::max(1e-3f * radius, radius - bounding_radius),
                          radius + bounding_radius);

  shaders.apply([this](opengl::shader_program_handle shader) {
    shader.bind()
        .set("projection", camera.projection_matrix())
        .set("view", camera.view_matrix())
        .try_set("viewport", camera.viewport_matrix());
  });
}

void viewer::update() {
  handle_surface_load_task();
  if (view_should_update) {
    update_view();
    view_should_update = false;
  }

  if (surface_should_update) {
    surface.update();
    compute_heat_data();
    surface_should_update = false;
  }

  shaders.reload([this](opengl::shader_program_handle shader) {
    shader.bind()
        .set("projection", camera.projection_matrix())
        .set("view", camera.view_matrix())
        .try_set("viewport", camera.viewport_matrix());
  });
}

void viewer::render() {
  glEnable(GL_SCISSOR_TEST);
  glScissor(camera.screen_offset_x(), camera.screen_offset_y(),
            camera.screen_width(), camera.screen_height());
  glViewport(camera.screen_offset_x(), camera.screen_offset_y(),
             camera.screen_width(), camera.screen_height());
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClearDepth(1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glDepthFunc(GL_LEQUAL);
  // surface_shader.bind();
  shaders.names["flat"]->second.shader.bind();
  // surface.render();
  surface.render();

  // shaders.names["contours"]->second.shader.bind();
  // surface.render();

  glDepthFunc(GL_ALWAYS);

  shaders.names["initial"]->second.shader.bind();
  device_initial_line.render();
  glDrawArrays(GL_LINE_STRIP, 0, device_initial_line.vertices.size());

  if (smooth_line_drawing) {
    shaders.names["points"]->second.shader.bind();
    // device_line.render();
    device_line.device_handle.bind();
    glDrawArrays(GL_LINE_STRIP, 0, device_line.vertices.size());
  }

  glDisable(GL_SCISSOR_TEST);
}

void viewer::set_view_should_update() noexcept {
  view_should_update = true;
}

void viewer::turn(const vec2& angle) {
  altitude += angle.y;
  azimuth += angle.x;
  constexpr float bound = pi / 2 - 1e-5f;
  altitude = std::clamp(altitude, -bound, bound);
  view_should_update = true;
}

void viewer::shift(const vec2& pixels) {
  const auto shift = -pixels.x * camera.right() + pixels.y * camera.up();
  const auto scale = camera.pixel_size() * radius;
  origin += scale * shift;
  view_should_update = true;
}

void viewer::zoom(float scale) {
  radius *= exp(-scale);
  view_should_update = true;
}

void viewer::look_at(float x, float y) {
  const auto r = camera.primary_ray(x, y);
  if (const auto p = intersection(r, surface)) {
    origin = r(p.t);
    radius = p.t;
    view_should_update = true;
  }
}

void viewer::set_z_as_up() {
  right = {1, 0, 0};
  front = {0, -1, 0};
  up = {0, 0, 1};
  view_should_update = true;
}

void viewer::set_y_as_up() {
  right = {1, 0, 0};
  front = {0, 0, 1};
  up = {0, 1, 0};
  view_should_update = true;
}

void viewer::load_surface(const filesystem::path& path) {
  try {
    const auto load_start = clock::now();
    surface.host() = polyhedral_surface_from(path);
    const auto load_end = clock::now();

    // Evaluate loading and processing time.
    surface_load_time = duration<float32>(load_end - load_start).count();

    // surface.update();
    fit_view();
    // print_surface_info();
    compute_topology_and_geometry();

    surface_should_update = true;

    info("Sucessfully loaded surface mesh from file.\nfile = '{}'",
         path.string());

  } catch (exception& e) {
    error("Failed to load surface mesh from file.\n{}\nfile = '{}'",  //
          e.what(), path.string());
    return;
  }
}

void viewer::async_load_surface(const filesystem::path& path) {
  if (surface_load_task.valid()) {
    error(
        "Failed to start asynchronous loading of surface mesh from file.\nIt "
        "seems another surface mesh is already loading.\n file = '{}'",
        path.string());
    return;
  }
  surface_load_task =
      async(launch::async, [this, &path] { load_surface(path); });
  info("Started to asynchronously load surface mesh from file.\nfile = '{}'",
       path.string());
}

void viewer::handle_surface_load_task() {
  if (!surface_load_task.valid()) return;
  if (future_status::ready != surface_load_task.wait_for(0s)) {
    // cout << "." << flush;
    return;
  }

  // info("Sucessfully finished to asynchronously load surface mesh.");

  surface_load_task = {};
}

void viewer::fit_view() {
  const auto box = aabb_from(surface);
  origin = box.origin();
  bounding_radius = box.radius();
  radius = bounding_radius / tan(0.5f * camera.vfov());
  camera.set_near_and_far(1e-4f * radius, 2 * radius);
  view_should_update = true;
}

void viewer::print_surface_info() {
  constexpr auto left_width = 20;
  constexpr auto right_width = 10;
  cout << setprecision(3) << fixed << boolalpha;
  cout << setw(left_width) << "load time"
       << " = " << setw(right_width) << surface_load_time << " s\n"
       << setw(left_width) << "process time"
       << " = " << setw(right_width) << surface_process_time << " s\n"
       << '\n';

  cout << setw(left_width) << "vertices"
       << " = " << setw(right_width) << surface.vertices.size() << '\n'
       << setw(left_width) << "faces"
       << " = " << setw(right_width) << surface.faces.size() << '\n'
       << endl;
}

void viewer::load_shader(const filesystem::path& path, const string& name) {
  shaders.load_shader(path);
  shaders.add_name(path, name);
}

void viewer::sort_surface_faces_by_depth() {
  auto faces = surface.faces;
  sort(begin(faces), end(faces), [&](const auto& f1, const auto& f2) {
    const auto& v = surface.vertices;
    const auto p1 =
        (v[f1[0]].position + v[f1[1]].position + v[f1[2]].position) / 3.0f;
    const auto d1 = length(camera.position() - p1);
    const auto p2 =
        (v[f2[0]].position + v[f2[1]].position + v[f2[2]].position) / 3.0f;
    const auto d2 = length(camera.position() - p2);
    return d1 > d2;
  });
  surface.device_faces.allocate_and_initialize(faces);
}

auto viewer::select_vertex(float x, float y) -> polyhedral_surface::vertex_id {
  const auto r = camera.primary_ray(x, y);
  const auto p = intersection(r, surface);
  if (!p) return polyhedral_surface::invalid;

  const auto& f = surface.faces[p.f];
  const auto w = real(1) - p.u - p.v;

  // const auto position = surface.vertices[f[0]].position * w +
  //                       surface.vertices[f[1]].position * u +
  //                       surface.vertices[f[2]].position * v;
  return f[0];
}

void viewer::select_origin_vertex(float x, float y) {
  device_line.vertices.clear();
  device_line.update();
  destination_vertex = polyhedral_surface::invalid;
  line_vids.clear();
  origin_vertex = select_vertex(x, y);
  if (origin_vertex == polyhedral_surface::invalid) return;
  // cout << "origin vid = " << origin_vertex << endl;
  line_vids.push_back(origin_vertex);
  selecting = true;
}

void viewer::select_destination_vertex(float x, float y) {
  if (!selecting) return;
  const auto vid = select_vertex(x, y);
  if (vid == polyhedral_surface::invalid) return;
  destination_vertex = vid;
  // cout << "destination vid = " << destination_vertex << endl;
  // compute_dijkstra_path();
  update_line();
  update_heat();
}

void viewer::compute_topology_and_geometry() {
  using namespace geometrycentral;
  using namespace surface;

  // Generate polygon data for constructors.
  //
  vector<vector<size_t>> polygons(surface.faces.size());
  for (size_t i = 0; const auto& f : surface.faces) {
    polygons[i].resize(3);
    for (size_t j = 0; j < 3; ++j) polygons[i][j] = f[j];
    ++i;
  }
  //
  mesh = make_unique<ManifoldSurfaceMesh>(polygons);

  // Generate vertex data for constructors.
  //
  VertexData<Vector3> vertices(*mesh);
  for (size_t i = 0; i < surface.vertices.size(); ++i) {
    vertices[i].x = surface.vertices[i].position.x;
    vertices[i].y = surface.vertices[i].position.y;
    vertices[i].z = surface.vertices[i].position.z;
  }
  //
  geometry = make_unique<VertexPositionGeometry>(*mesh, vertices);
}

void viewer::compute_dijkstra_path() {
  if ((origin_vertex == polyhedral_surface::invalid) ||
      (destination_vertex == polyhedral_surface::invalid) ||
      (origin_vertex == destination_vertex))
    return;

  using namespace geometrycentral;
  using namespace surface;

  const auto network = FlipEdgeNetwork::constructFromDijkstraPath(
      *mesh, *geometry, Vertex{mesh.get(), origin_vertex},
      Vertex{mesh.get(), destination_vertex});
  // network->iterativeShorten();
  network->posGeom = geometry.get();
  vector<Vector3> path = network->getPathPolyline3D().front();

  device_line.vertices.clear();
  for (const auto& v : path)
    device_line.vertices.push_back(vec3{real(v.x), real(v.y), real(v.z)});
  device_line.update();
}

void viewer::update_line() {
  if ((destination_vertex == polyhedral_surface::invalid) ||
      (origin_vertex == destination_vertex))
    return;

  using namespace geometrycentral;
  using namespace surface;

  // Get the shortest edge path by using
  // Dijkstra's Algorithm by Geometry Central.
  //
  const auto network = FlipEdgeNetwork::constructFromDijkstraPath(
      *mesh, *geometry, Vertex{mesh.get(), origin_vertex},
      Vertex{mesh.get(), destination_vertex});
  network->posGeom = geometry.get();
  const auto paths = network->getPathPolyline();

  // Check the path for consistency.
  //
  assert(paths.size() == 1);
  const auto& path = paths[0];
  assert(path.front().vertex.getIndex() == origin_vertex);
  assert(path.back().vertex.getIndex() == destination_vertex);

  // Store this path at the end of the current line.
  //
  for (size_t i = 1; i < path.size(); ++i)
    line_vids.push_back(path[i].vertex.getIndex());
  origin_vertex = destination_vertex;

  // Update the structure for line rendering.
  //
  device_initial_line.vertices.clear();
  for (auto vid : line_vids)
    device_initial_line.vertices.push_back(surface.vertices[vid].position);
  device_initial_line.update();
}

void viewer::shorten_line() {
  if (line_vids.size() <= 1) return;

  using namespace geometrycentral;
  using namespace surface;

  // Construct path of halfedges from vertex indices.
  // We have to do this anyway as the surface point data
  // structure does not provide correctly oriented halfedges.
  //
  vector<Halfedge> edges{};
  for (size_t i = 1; i < line_vids.size(); ++i) {
    Vertex p(mesh.get(), line_vids[i - 1]);
    Vertex q(mesh.get(), line_vids[i]);

    auto he = q.halfedge();
    while (he.tipVertex() != p) he = he.nextOutgoingNeighbor();
    // The halfedge must point to the previous point.
    // Otherwise, edges do not count as path for FlipEdgeNetwork construction.
    edges.push_back(he.twin());

    // cout << line_vids[i - 1] << " -> " << line_vids[i] << '\t'
    //      << he.tipVertex().getIndex() << "," << he.tailVertex().getIndex()
    //      << endl;
  }

  auto g = geometry.get();
  if (displacing) g = displaced_geometry.get();

  FlipEdgeNetwork network(*mesh, *g, {edges});
  network.iterativeShorten();
  network.posGeom = g;
  vector<Vector3> path = network.getPathPolyline3D().front();

  device_line.vertices.clear();
  for (const auto& v : path)
    device_line.vertices.push_back(vec3{real(v.x), real(v.y), real(v.z)});
  device_line.update();
}

void viewer::compute_heat_data() {
  // Construct vertex matrix.
  //
  surface_vertex_matrix.resize(surface.vertices.size(), 3);
  for (size_t i = 0; i < surface.vertices.size(); ++i)
    for (size_t j = 0; j < 3; ++j)
      surface_vertex_matrix(i, j) = surface.vertices[i].position[j];

  // Construct face matrix.
  //
  surface_face_matrix.resize(surface.faces.size(), 3);
  for (size_t i = 0; i < surface.faces.size(); ++i)
    for (size_t j = 0; j < 3; ++j)
      surface_face_matrix(i, j) = surface.faces[i][j];

  // Construct heat data.
  //
  auto t =
      pow(igl::avg_edge_length(surface_vertex_matrix, surface_face_matrix), 2);
  if (!igl::heat_geodesics_precompute(surface_vertex_matrix,
                                      surface_face_matrix, t, heat_data)) {
    cerr << "ERROR: Precomputation of heat data failed." << endl;
    exit(1);
  }

  potential.assign(surface.vertices.size(), 0);
  device_heat.allocate_and_initialize(potential);
}

void viewer::update_heat() {
  heat = Eigen::VectorXd::Zero(surface_vertex_matrix.rows());
  Eigen::VectorXi gamma(line_vids.size());
  for (size_t i = 0; i < line_vids.size(); ++i) gamma[i] = line_vids[i];

  igl::heat_geodesics_solve(heat_data, gamma, heat);

  potential.assign(heat.size(), 0);
  double max_heat = 0;
  for (size_t i = 0; i < heat.size(); ++i)
    max_heat = std::max(max_heat, heat[i]);
  for (size_t i = 0; i < potential.size(); ++i)
    potential[i] = heat[i] / max_heat;
  const auto modifier = [this](auto x) {
    return (x <= 1e-4f) ? 0 : exp(-1.0f / tolerance / x);
  };
  for (size_t i = 0; i < potential.size(); ++i)
    potential[i] = modifier(potential[i]);
  device_heat.allocate_and_initialize(potential);

  // Generate vertex data for constructors.
  //
  using namespace geometrycentral;
  using namespace surface;
  EdgeData<double> edge_lengths(*mesh);
  for (auto e : mesh->edges()) {
    const auto vid1 = e.halfedge().tipVertex().getIndex();
    const auto vid2 = e.halfedge().tailVertex().getIndex();
    const auto squared = [](auto x) { return x * x; };
    edge_lengths[e] = sqrt(length2(surface.vertices[vid1].position -
                                   surface.vertices[vid2].position) +
                           squared(potential[vid1] - potential[vid2]));
  }
  //
  lifted_geometry = make_unique<EdgeLengthGeometry>(*mesh, edge_lengths);
}

void viewer::add_normal_displacement() {
  auto vertices = surface.vertices;
  for (size_t i = 0; auto& v : vertices) {
    v.position += 0.5f * bounding_radius * potential[i] * v.normal;
    ++i;
  }
  surface.device_vertices.allocate_and_initialize(vertices);

  // Generate vertex data for constructors.
  //
  using namespace geometrycentral;
  using namespace surface;
  VertexData<Vector3> geometry_vertices(*mesh);
  for (size_t i = 0; i < vertices.size(); ++i) {
    geometry_vertices[i].x = vertices[i].position.x;
    geometry_vertices[i].y = vertices[i].position.y;
    geometry_vertices[i].z = vertices[i].position.z;
  }
  //
  displaced_geometry =
      make_unique<VertexPositionGeometry>(*mesh, geometry_vertices);

  displacing = true;
}

void viewer::remove_normal_displacement() {
  surface.device_vertices.allocate_and_initialize(surface.vertices);
  displacing = false;
}

void viewer::smooth_line() {
  if (line_vids.size() <= 1) return;

  using namespace geometrycentral;
  using namespace surface;

  // Construct path of halfedges from vertex indices.
  // We have to do this anyway as the surface point data
  // structure does not provide correctly oriented halfedges.
  //
  vector<Halfedge> edges{};
  for (size_t i = 1; i < line_vids.size(); ++i) {
    Vertex p(mesh.get(), line_vids[i - 1]);
    Vertex q(mesh.get(), line_vids[i]);

    auto he = q.halfedge();
    while (he.tipVertex() != p) he = he.nextOutgoingNeighbor();
    // The halfedge must point to the previous point.
    // Otherwise, edges do not count as path for FlipEdgeNetwork construction.
    edges.push_back(he.twin());

    // cout << line_vids[i - 1] << " -> " << line_vids[i] << '\t'
    //      << he.tipVertex().getIndex() << "," << he.tailVertex().getIndex()
    //      << endl;
  }

  FlipEdgeNetwork network(*mesh, *lifted_geometry, {edges});
  network.iterativeShorten();
  network.posGeom = geometry.get();
  vector<Vector3> path = network.getPathPolyline3D().front();

  device_line.vertices.clear();
  for (const auto& v : path)
    device_line.vertices.push_back(vec3{real(v.x), real(v.y), real(v.z)});
  device_line.update();
}

}  // namespace hyperreflex
