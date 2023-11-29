#version 330 core

layout (location = 0) out vec4 frag_color;

void main() {
  frag_color = vec4(0.1, 0.5, 0.9, 0.7);
  // frag_color = vec4(1.0, 0.7, 0.3, 1.0);
}
