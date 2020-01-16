#version 410

layout (triangles, equal_spacing, cw) in;

layout (location = 0) in vec3 te_pos[];
layout (location = 1) in vec3 te_norm[];

out vec3 fs_norm; // Vertex normal in camera coords.
out vec3 fs_pos; // Vertex position in camera coords

uniform mat4 modelviewmatrix;
uniform mat4 projectionmatrix;
uniform mat3 normalmatrix;

vec3 edgeCP(vec3 e, vec3 v0, vec3 v1){
    return (e * 4.0 - v0 - v1) * 0.5;
}

vec3 Q(float u, float v, float w, vec3 v0, vec3 v1, vec3 v2, vec3 e0, vec3 e1, vec3 e2){
    float b200 = pow(u,2);
    float b020 = pow(v,2);
    float b002 = pow(w,2);

    float b110 = 2*u*v;
    float b101 = 2*u*w;
    float b011 = 2*v*w;

    vec3 c200 = v0;
    vec3 c020 = v1;
    vec3 c002 = v2;

    vec3 c110 = edgeCP(e0, v0, v1);
    vec3 c101 = edgeCP(e2, v0, v2);
    vec3 c011 = edgeCP(e1, v1, v2);

    return c110 * u + c101 * v + c011 * w;
    //return b200 * c200 + b020 * c020 + b002 * c002 + b110 * c110 + b101 * c101 + b011 * c011;
}

vec3 P(float u, float v, float w, vec3 p0, vec3 p1, vec3 p2, vec3 pe0, vec3 pe1, vec3 pe2){
    return Q(u, v, w, p0, p1, p2, pe0, pe1, pe2);
}

vec3 N(float u, float v, float w, vec3 n0, vec3 n1, vec3 n2, vec3 ne0, vec3 ne1, vec3 ne2){
    return Q(u, v, w, n0, n1, n2, ne0, ne1, ne2);
}

void main(void)
{
    // The tessellation u,v,w coordinate
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    float w = 1.0 - u - v;

    // The patch vertices (control points)
    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[2].gl_Position.xyz;
    vec3 p2 = gl_in[4].gl_Position.xyz;
    vec3 pe0 = gl_in[1].gl_Position.xyz;
    vec3 pe1 = gl_in[3].gl_Position.xyz;
    vec3 pe2 = gl_in[5].gl_Position.xyz;

    vec3 n0 = te_norm[0];
    vec3 n1 = te_norm[2];
    vec3 n2 = te_norm[4];
    vec3 ne0 = te_norm[1];
    vec3 ne1 = te_norm[3];
    vec3 ne2 = te_norm[5];

    // Interpolation
    //fs_pos = P(u, v, w, p0, p1, p2, pe0, pe1, pe2);
    fs_norm = N(u, v, w, n0, n1, n2, ne0, ne1, ne2);    

    fs_pos = p0 * u + pe0 * v + p1 * w;

    // Transform to clip coordinates
    gl_Position = projectionmatrix * modelviewmatrix * vec4(fs_pos, 1.0);

    // Convert to camera coordinates
    fs_pos = vec3(modelviewmatrix * vec4(fs_pos, 1.0));
    fs_norm = normalize(normalmatrix * fs_norm);

}
