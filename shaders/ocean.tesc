#version 450

//TODO: implement ocean tesselation control shader
// see if use of quads or triangles
// implementation done for quads here

layout(location = 0) in vec3 in_normal[];
layout(location = 1) in vec2 in_uv[];
layout(location = 2) in vec3 in_color[];
layout(location = 4) in vec3 in_tangent[];
layout(location = 5) in vec3 in_bitangent[];

layout(location = 0) out vec3 out_normal[];
layout(location = 1) out vec2 out_uv[];
layout(location = 2) out vec3 out_color[];
layout(location = 4) out vec3 out_tangent[];
layout(location = 5) out vec3 out_bitangent[];

layout (vertices = 4) out;

uniform float tesselation_level;

void main() {

    gl_TessLevelOuter[0] = tesselation_level;
    gl_TessLevelOuter[1] = tesselation_level;
    gl_TessLevelOuter[2] = tesselation_level;
    gl_TessLevelOuter[3] = tesselation_level;
    gl_TessLevelInner[0] = tesselation_level;
    gl_TessLevelInner[1] = tesselation_level;

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    out_normal[gl_InvocationID] = in_normal[gl_InvocationID];
    out_uv[gl_InvocationID] = in_uv[gl_InvocationID];
    out_color[gl_InvocationID] = in_color[gl_InvocationID];
    out_tangent[gl_InvocationID] = out_tangent[gl_InvocationID];
    out_bitangent[gl_InvocationID] = in_bitangent[gl_InvocationID];
}