#version 420 core

uniform int viewport_width;
uniform int viewport_height;

layout (lines) in;
layout (triangle_strip, max_vertices = 12) out;

noperspective out vec2 uv;

void main(){
  float width = 10.0;

  vec4 pos1 = gl_in[0].gl_Position / gl_in[0].gl_Position.w;
  vec4 pos2 = gl_in[1].gl_Position / gl_in[1].gl_Position.w;

  vec2 p = vec2(0.5 * viewport_width * pos1.x, 0.5 * viewport_height * pos1.y);
  vec2 q = vec2(0.5 * viewport_width * pos2.x, 0.5 * viewport_height * pos2.y);

  vec2 d = normalize(q - p);
  vec2 n = vec2(-d.y, d.x);
  float delta = 0.5 * width;

  vec2 t = vec2(0);

  t = p - delta * n;
  uv = vec2(0.0, -1.0);
  gl_Position = vec4(2.0 * t.x / viewport_width,
                     2.0 * t.y / viewport_height,
                     pos1.z, 1.0);
  EmitVertex();
  t = p + delta * n;
  uv = vec2(0.0, 1.0);
  gl_Position = vec4(2.0 * t.x / viewport_width,
                     2.0 * t.y / viewport_height,
                     pos1.z, 1.0);
  EmitVertex();
  t = p - delta * n - delta * d;
  uv = vec2(-1.0, -1.0);
  gl_Position = vec4(2.0 * t.x / viewport_width,
                     2.0 * t.y / viewport_height,
                     pos1.z, 1.0);
  EmitVertex();
  t = p + delta * n - delta * d;
  uv = vec2(-1.0, 1.0);
  gl_Position = vec4(2.0 * t.x / viewport_width,
                     2.0 * t.y / viewport_height,
                     pos1.z, 1.0);
  EmitVertex();
  EndPrimitive();

  t = q - delta * n;
  uv = vec2(0.0, -1.0);
  gl_Position = vec4(2.0 * t.x / viewport_width,
                     2.0 * t.y / viewport_height,
                     pos2.z, 1.0);
  EmitVertex();
  t = q + delta * n;
  uv = vec2(0.0, 1.0);
  gl_Position = vec4(2.0 * t.x / viewport_width,
                     2.0 * t.y / viewport_height,
                     pos2.z, 1.0);
  EmitVertex();
  t = q - delta * n + delta * d;
  uv = vec2(1.0, -1.0);
  gl_Position = vec4(2.0 * t.x / viewport_width,
                     2.0 * t.y / viewport_height,
                     pos2.z, 1.0);
  EmitVertex();
  t = q + delta * n + delta * d;
  uv = vec2(1.0, 1.0);
  gl_Position = vec4(2.0 * t.x / viewport_width,
                     2.0 * t.y / viewport_height,
                     pos2.z, 1.0);
  EmitVertex();
  EndPrimitive();


  t = p - delta * n;
  uv = vec2(0.0, -1.0);
  gl_Position = vec4(2.0 * t.x / viewport_width,
                     2.0 * t.y / viewport_height,
                     pos1.z, 1.0);
  EmitVertex();
  t = q - delta * n;
  uv = vec2(0.0, -1.0);
  gl_Position = vec4(2.0 * t.x / viewport_width,
                     2.0 * t.y / viewport_height,
                     pos2.z, 1.0);
  EmitVertex();
  t = p + delta * n;
  uv = vec2(0.0, 1.0);
  gl_Position = vec4(2.0 * t.x / viewport_width,
                     2.0 * t.y / viewport_height,
                     pos1.z, 1.0);
  EmitVertex();
  t = q + delta * n;
  uv = vec2(0.0, 1.0);
  gl_Position = vec4(2.0 * t.x / viewport_width,
                     2.0 * t.y / viewport_height,
                     pos2.z, 1.0);
  EmitVertex();
  EndPrimitive();
}
