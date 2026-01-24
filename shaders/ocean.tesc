#version 450

#include "utils.glsl"

//TODO: implement ocean tesselation control shader
// see if use of quads or triangles
// implementation done for quads here

layout (vertices = 4) out;

void main() {

    gl_TessLevelOuter[0] = 10.0;
    gl_TessLevelOuter[1] = 10.0;
    gl_TessLevelOuter[2] = 10.0;
    gl_TessLevelOuter[3] = 10.0;
    gl_TessLevelInner[0] = 10.0;
    gl_TessLevelInner[1] = 10.0;

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}