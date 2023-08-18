#version 330 core

in vec3 pos;
in vec3 nor;
in float hea;
noperspective in vec3 edge_distance;

layout(location = 0) out vec4 frag_color;

float colormap_f(float x, float phase) {
  const float pi = 3.141592653589793238462643383279502884197169399;
  const float a = 126.9634465941118;
  const float b = 1.011727672706345;
  const float c = 0.0038512319231245;
  const float d = 127.5277540583575;
  return a * sin(2.0 * pi / b * x + 2.0 * pi * (c + phase)) + d;
}

float colormap_red(float x) {
  return colormap_f(x, 0.5);
}

float colormap_green(float x) {
  const float pi = 3.141592653589793238462643383279502884197169399;
  const float a = 63.19460736097507;
  const float b = 0.06323746667143024;
  const float c = 0.06208443629833329;
  const float d = 96.56305326777574;
  return a * sin(2.0 * pi / b * x + 2.0 * pi * c) + d;
}

float colormap_blue(float x) {
  return colormap_f(x, 0.0);
}

vec4 colormap(float x) {
  float r = clamp(colormap_red(x) / 255.0, 0.0, 1.0);
  float g = clamp(colormap_green(x) / 255.0, 0.0, 1.0);
  float b = clamp(colormap_blue(x) / 255.0, 0.0, 1.0);
  return vec4(r, g, b, 1.0);
}

void main() {
  // Compute distance from edges.
  float d = min(edge_distance.x, edge_distance.y);
  d = min(d, edge_distance.z);
  float line_width = 0.05;
  float line_delta = 1.0;
  float alpha = 1.0;
  vec4 line_color = vec4(vec3(0.2), 1.0);
  float mix_value =
      smoothstep(line_width - line_delta, line_width + line_delta, d);
  // Compute viewer shading.
  float s = abs(normalize(nor).z);
  float light = 0.2 + 1.0 * pow(s, 1000) + 0.75 * pow(s, 0.2);
  // float light = 0.2 + 0.75 * pow(s, 0.2);
  // vec4 light_color = vec4(vec3(light), 1.0);
  // Mix both color values.
  vec4 light_color = colormap(hea);
  frag_color = mix(line_color, light_color, mix_value);
  // if (mix_value > 0.9) discard;
  //   frag_color = (1 - mix_value) * line_color;
}
