#version 420 core

noperspective in vec2 uv;

layout(location = 0) out vec4 frag_color;

void main(){
  if (length(uv) >= 1.0) discard;
  frag_color = vec4(vec3(0.4), 1.0);
}
