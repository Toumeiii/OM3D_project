#version 450

#include "utils.glsl"

// TODO: implement ocean fragment shader
// maybe lighting and transparency

layout(location = 4) in vec3 in_color;

layout(location = 2) out vec3 out_color;



void main() {
    out_color = in_color;
}