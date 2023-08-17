#include <hyperreflex/viewer.hpp>
//
#include <hyperreflex/math.hpp>

namespace hyperreflex {

viewer_context::viewer_context() {
  sf::ContextSettings settings;
  settings.majorVersion = 4;
  settings.minorVersion = 5;
  // These values need to be set when 3D rendering is required.
  settings.depthBits = 24;
  settings.stencilBits = 8;
  settings.antialiasingLevel = 4;

  window.create(sf::VideoMode(800, 450), "hyperreflex", sf::Style::Default,
                settings);
  window.setVerticalSyncEnabled(true);
  window.setKeyRepeatEnabled(false);
  window.setActive(true);

  glbinding::initialize(sf::Context::getFunction);
}

viewer::viewer() : viewer_context() {
  // To initialize the viewport and matrices,
  // window has to be resized at least once.
  resize();

  // Setup for OpenGL
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH_TEST);
  // glEnable(GL_POINT_SMOOTH);
  // glEnable(GL_POINT_SPRITE);
  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glPointSize(10.0f);
  glLineWidth(4.0f);

  surface.setup();
}

void viewer::resize() {
  const auto s = window.getSize();
  resize(s.x, s.y);
}

void viewer::resize(int width, int height) {
  glViewport(0, 0, width, height);
  cam.set_screen_resolution(width, height);
  view_should_update = true;
}

void viewer::process_events() {
  // Get new mouse position and compute movement in space.
  const auto new_mouse_pos = sf::Mouse::getPosition(window);
  const auto mouse_move = new_mouse_pos - mouse_pos;
  mouse_pos = new_mouse_pos;

  sf::Event event;
  while (window.pollEvent(event)) {
    if (event.type == sf::Event::Closed)
      running = false;
    else if (event.type == sf::Event::Resized)
      resize(event.size.width, event.size.height);
    else if (event.type == sf::Event::MouseWheelScrolled)
      zoom(0.1 * event.mouseWheelScroll.delta);
    else if (event.type == sf::Event::MouseButtonPressed) {
      switch (event.mouseButton.button) {
        case sf::Mouse::Middle:
          look_at(event.mouseButton.x, event.mouseButton.y);
          break;
      }
    } else if (event.type == sf::Event::KeyPressed) {
      switch (event.key.code) {
        case sf::Keyboard::Escape:
          running = false;
          break;
        case sf::Keyboard::Num1:
          set_y_as_up();
          break;
        case sf::Keyboard::Num2:
          set_z_as_up();
          break;
        case sf::Keyboard::N:
          expand_selection();
          break;
        case sf::Keyboard::X:
          group = (group + 1) % surface.component_count();
          select_component();
          break;
        case sf::Keyboard::Y:
          group = (group + surface.component_count() - 1) %
                  surface.component_count();
          select_component();
          break;
        case sf::Keyboard::Z:
          sort_surface_faces_by_depth();
          break;
      }
    }
  }

  if (window.hasFocus()) {
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
        shift({mouse_move.x, mouse_move.y});
      else
        turn({-0.01 * mouse_move.x, 0.01 * mouse_move.y});
    }

    if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
      if (mouse_move != sf::Vector2i{}) {
      }
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
    }
  }
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
  cam.move(p).look_at(origin, up);

  cam.set_near_and_far(std::max(1e-3f * radius, radius - bounding_radius),
                       radius + bounding_radius);

  shaders.apply([this](opengl::shader_program_handle shader) {
    shader.bind()
        .set("projection", cam.projection_matrix())
        .set("view", cam.view_matrix())
        .try_set("viewport", cam.viewport_matrix());
  });
}

void viewer::update() {
  handle_surface_load_task();
  if (view_should_update) {
    update_view();
    view_should_update = false;
  }

  shaders.reload([this](opengl::shader_program_handle shader) {
    shader.bind()
        .set("projection", cam.projection_matrix())
        .set("view", cam.view_matrix())
        .try_set("viewport", cam.viewport_matrix());
  });
}

void viewer::render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glDepthFunc(GL_LEQUAL);
  // surface_shader.bind();
  shaders.names["flat"]->second.shader.bind();
  // surface.render();
  surface.render();

  // shaders.names["contours"]->second.shader.bind();
  // surface.render();

  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  // selection_shader.bind();

  surface.device_handle.bind();

  shaders.names["selection"]->second.shader.bind();
  selection.bind();
  glDrawElements(GL_TRIANGLES, 3 * selection.size() / sizeof(GL_UNSIGNED_INT),
                 GL_UNSIGNED_INT, 0);

  shaders.names["boundary"]->second.shader.bind();
  surface_boundary.bind();
  glDrawElements(GL_LINES, surface_boundary.size() / sizeof(GL_UNSIGNED_INT),
                 GL_UNSIGNED_INT, 0);

  glDepthFunc(GL_ALWAYS);

  shaders.names["unoriented"]->second.shader.bind();
  surface_unoriented_edges.bind();
  glDrawElements(GL_LINES,
                 surface_unoriented_edges.size() / sizeof(GL_UNSIGNED_INT),
                 GL_UNSIGNED_INT, 0);

  shaders.names["inconsistent"]->second.shader.bind();
  surface_inconsistent_edges.bind();
  glDrawElements(GL_LINES,
                 surface_inconsistent_edges.size() / sizeof(GL_UNSIGNED_INT),
                 GL_UNSIGNED_INT, 0);

  // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // edge_selection.bind();
  // glDrawElements(GL_LINES, edge_selection.size(), GL_UNSIGNED_INT, 0);
}

void viewer::run() {
  running = true;
  while (running) {
    process_events();
    update();
    render();
    window.display();
  }
}

void viewer::turn(const vec2& angle) {
  altitude += angle.y;
  azimuth += angle.x;
  constexpr float bound = pi / 2 - 1e-5f;
  altitude = std::clamp(altitude, -bound, bound);
  view_should_update = true;
}

void viewer::shift(const vec2& pixels) {
  const auto shift = -pixels.x * cam.right() + pixels.y * cam.up();
  const auto scale = cam.pixel_size() * radius;
  origin += scale * shift;
  view_should_update = true;
}

void viewer::zoom(float scale) {
  radius *= exp(-scale);
  view_should_update = true;
}

void viewer::look_at(float x, float y) {
  const auto r = cam.primary_ray(x, y);
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
  const auto loader = [this](const filesystem::path& path) {
    try {
      const auto load_start = clock::now();
      surface.host() = polyhedral_surface_from(path);
      const auto load_end = clock::now();

      cout << "loaded" << endl;

      const auto preprocess_start = clock::now();
      surface.generate_topological_structure();
      const auto preprocess_end = clock::now();

      // Evaluate loading and processing time.
      surface_load_time = duration<float32>(load_end - load_start).count();
      surface_process_time =
          duration<float32>(preprocess_end - preprocess_start).count();
    } catch (exception& e) {
      cout << "failed.\n" << e.what() << endl;
      return;
    }
  };
  surface_load_task = async(launch::async, loader, path);
  cout << "Loading " << path << "..." << endl;
}

void viewer::handle_surface_load_task() {
  if (!surface_load_task.valid()) return;
  if (future_status::ready != surface_load_task.wait_for(0s)) {
    // cout << "." << flush;
    return;
  }
  cout << "done." << endl << '\n';
  surface_load_task = {};

  surface.update();
  fit_view();
  print_surface_info();

  vector<uint32> lines{};
  for (size_t fid = 0; fid < surface.faces.size(); ++fid) {
    if (surface.face_adjacencies[fid][0] == polyhedral_surface::invalid) {
      lines.push_back(surface.faces[fid][0]);
      lines.push_back(surface.faces[fid][1]);
    }
    if (surface.face_adjacencies[fid][1] == polyhedral_surface::invalid) {
      lines.push_back(surface.faces[fid][1]);
      lines.push_back(surface.faces[fid][2]);
    }
    if (surface.face_adjacencies[fid][2] == polyhedral_surface::invalid) {
      lines.push_back(surface.faces[fid][2]);
      lines.push_back(surface.faces[fid][0]);
    }
  }
  surface_boundary.allocate_and_initialize(lines);

  lines.clear();
  for (const auto& [e, info] : surface.edges) {
    if (info.oriented()) continue;
    const auto e2 = surface.common_edge(info.face[0], info.face[1]);
    lines.push_back(e2[0]);
    lines.push_back(e2[1]);
  }
  surface_unoriented_edges.allocate_and_initialize(lines);

  lines.clear();
  for (const auto& [e, info] : surface.edges) {
    if (info.oriented() || !surface.edges.contains({e[1], e[0]})) continue;
    const auto e2 = surface.common_edge(info.face[0], info.face[1]);
    lines.push_back(e2[0]);
    lines.push_back(e2[1]);
  }
  surface_inconsistent_edges.allocate_and_initialize(lines);
}

void viewer::fit_view() {
  const auto box = aabb_from(surface);
  origin = box.origin();
  bounding_radius = box.radius();
  radius = bounding_radius / tan(0.5f * cam.vfov());
  cam.set_near_and_far(1e-4f * radius, 2 * radius);
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
       << setw(left_width) << "consistent"
       << " = " << setw(right_width) << surface.consistent() << '\n'
       << setw(left_width) << "oriented"
       << " = " << setw(right_width) << surface.oriented() << '\n'
       << setw(left_width) << "boundary"
       << " = " << setw(right_width) << surface.has_boundary() << '\n'
       << setw(left_width) << "components"
       << " = " << setw(right_width) << surface.component_count() << '\n'
       << endl;
}

void viewer::load_shader(const filesystem::path& path, const string& name) {
  shaders.load_shader(path);
  shaders.add_name(path, name);
}

void viewer::update_selection() {
  decltype(surface.faces) faces{};
  for (size_t i = 0; i < selected_faces.size(); ++i)
    if (selected_faces[i]) faces.push_back(surface.faces[i]);
  selection.allocate_and_initialize(faces);
}

void viewer::select_face(float x, float y) {
  selected_faces.resize(surface.faces.size());
  for (size_t i = 0; i < selected_faces.size(); ++i) selected_faces[i] = false;

  if (const auto p = intersection(cam.primary_ray(x, y), surface)) {
    selected_faces[p.f] = true;
    update_selection();
  }
}

void viewer::expand_selection() {
  auto new_selected_faces = selected_faces;
  for (size_t i = 0; i < selected_faces.size(); ++i) {
    if (!selected_faces[i]) continue;
    for (int j = 0; j < 3; ++j) {
      if (surface.face_adjacencies[i][j] == scene::invalid) continue;
      new_selected_faces[surface.face_adjacencies[i][j]] = true;
    }
  }
  swap(new_selected_faces, selected_faces);
  update_selection();
}

void viewer::select_component() {
  // selected_faces.resize(surface.faces.size());
  // for (size_t i = 0; i < surface.faces.size(); ++i)
  //   selected_faces[i] = (surface.face_component[i] == group);
  // update_selection();
  // for (auto fid : surface.component_faces(group))

  const auto r = surface.component_faces(group);
  decltype(surface.faces) faces(ranges::begin(r), ranges::end(r));
  selection.allocate_and_initialize(faces);
}

void viewer::sort_surface_faces_by_depth() {
  auto faces = surface.faces;
  sort(begin(faces), end(faces), [&](const auto& f1, const auto& f2) {
    const auto& v = surface.vertices;
    const auto p1 =
        (v[f1[0]].position + v[f1[1]].position + v[f1[2]].position) / 3.0f;
    const auto d1 = length(cam.position() - p1);
    const auto p2 =
        (v[f2[0]].position + v[f2[1]].position + v[f2[2]].position) / 3.0f;
    const auto d2 = length(cam.position() - p2);
    return d1 > d2;
  });
  surface.device_faces.allocate_and_initialize(faces);
}

}  // namespace hyperreflex
