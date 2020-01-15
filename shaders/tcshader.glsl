#version 410

layout (vertices = 3) out;

layout (location = 0) in vec3 tc_pos[];
layout (location = 1) in vec3 tc_norm[];

layout (location = 0) out vec3 te_pos[];
layout (location = 1) out vec3 te_norm[];

void main(void)
{
    // Pass along the vertex position unmodified
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    te_pos[gl_InvocationID] = tc_pos[gl_InvocationID];
    te_norm[gl_InvocationID] = tc_norm[gl_InvocationID];

    if (gl_InvocationID == 0)
    {
        gl_TessLevelInner[0] = 1.0;
        gl_TessLevelOuter[0] = 3.0;
        gl_TessLevelOuter[1] = 3.0;
        gl_TessLevelOuter[2] = 3.0;
    }
}
