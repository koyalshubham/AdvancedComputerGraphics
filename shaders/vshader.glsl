#version 410
// Vertex shader

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;

layout (location = 0) out vec3 tc_pos;
layout (location = 1) out vec3 tc_norm;

void main() {
  gl_Position = vec4(pos, 1.0);

  tc_pos = pos;
  tc_norm = norm;
}
