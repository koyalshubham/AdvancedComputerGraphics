#version 410

layout(location = 0) in vec4 inPosition;
layout(location = 1) in float inSize;
layout(location = 2) in vec4 inColor;

// Per-quad output variables.
out Quad
{
    float size;
    vec4 color;
} outQuad;

void main()
{
    outQuad.size = inSize;
    outQuad.color = inColor;
    // Pass position along to the next stage. The actual work is done in the
    // Tessellation Evaluation shader.
    gl_Position = inPosition;
}
