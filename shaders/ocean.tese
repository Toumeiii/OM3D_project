#version 450

#include "utils.glsl"

// TODO: implement ocean tesselation evaluation shader
// implement different octave waves and displacement here
// implement cascade approch for waves
// implment Jacobien for normal calculation
// chose a M between 0.3 and 0.5 as threshold for foam generation -> color

layout (quads, equal_spacing, ccw) in;

out vec3 tes_color;

void main() {
    vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
    vec4 p2 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
    vec4 p = mix(p1, p2, gl_TessCoord.y);

    tes_color = vec3(0.0, 0.3, 0.5); // Placeholder color for ocean

    gl_Position = projection_matrix * model_view_matrix * p;
}