#version 420 core

// uniform sampler2D head_pointer_image;
// uniform samplerBuffer list_buffer;
layout (binding = 0, rgba32ui) uniform uimageBuffer list_buffer;
layout (binding = 1, r32ui) uniform uimage2DRect head_pointer_image;

#define MAX_FRAGMENTS 15

uvec4 fragments[MAX_FRAGMENTS];

layout (location = 0) out vec4 output_color;

int build_local_fragment_list(){
  uint current;
  int frag_count = 0;
  // current = texelFetch(head_pointer_image, ivec2(gl_FragCoord.xy), 0);
  current = uint(imageLoad(head_pointer_image, ivec2(gl_FragCoord.xy)));
  while (current != 0xffffffff && frag_count < MAX_FRAGMENTS){
    // item = texelFetch(list_buffer, current);
    uvec4 item = imageLoad(list_buffer, int(current));
    current = item.x;
    fragments[frag_count] = item;
    ++frag_count;
  }
  return frag_count;
}

void sort_fragment_list(int frag_count){
  for (int i = 0; i < frag_count; ++i){
    for (int j = i+1; j < frag_count; ++j){
      float depth_i = uintBitsToFloat(fragments[i].z);
      float depth_j = uintBitsToFloat(fragments[j].z);
      if (depth_i > depth_j){
        uvec4 tmp = fragments[i];
        fragments[i] = fragments[j];
        fragments[j] = tmp;
      }
    }
  }
}

vec4 blend(vec4 current_color, vec4 new_color){
  return mix(current_color, new_color, new_color.a);
}

vec4 calculate_final_color(int frag_count){
  vec4 final_color = vec4(0.0);
  for (int i = 0; i < frag_count; ++i){
    vec4 frag_color = unpackUnorm4x8(fragments[i].y);
    final_color = blend(final_color, frag_color);
  }
  return final_color;
}

void main(){
  int frag_count;
  frag_count = build_local_fragment_list();
  sort_fragment_list(frag_count);
  output_color = calculate_final_color(frag_count);
}
