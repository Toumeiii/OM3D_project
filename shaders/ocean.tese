#version 450

#include "utils.glsl"

// TODO: implement ocean tesselation evaluation shader
// implement different octave waves and displacement here
// implement cascade approch for waves
// implment Jacobien for normal calculation
// chose a M between 0.3 and 0.5 as threshold for foam generation -> color

layout(location = 0) in vec3 in_normal[];
layout(location = 1) in vec2 in_uv[];
layout(location = 2) in vec3 in_color[];
layout(location = 4) in vec3 in_tangent[];
layout(location = 5) in vec3 in_bitangent[];

layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec3 out_color;
layout(location = 3) out vec3 out_position;
layout(location = 4) out vec3 out_tangent;
layout(location = 5) out vec3 out_bitangent;

layout (quads, equal_spacing, ccw) in;

layout(binding = 0) uniform Data {
    FrameData frame;
};

layout (binding = 6)uniform sampler2D waves[4];

const float m = 0.4; // foam threshold
const float k = 2.0; // foam gain

vec4 mix_4_values(vec4 a, vec4 b, vec4 c, vec4 d) {
    vec4 v1 = mix(a, b, gl_TessCoord.x);
    vec4 v2 = mix(d, c, gl_TessCoord.x);
    return mix(v1, v2, gl_TessCoord.y);
}

void main() {
    vec4 p = mix_4_values(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position, gl_in[3].gl_Position);
    out_position = p.xyz;
    out_normal = mix_4_values(vec4(in_normal[0], 0.), vec4(in_normal[1], 0.), vec4(in_normal[2], 0.), vec4(in_normal[3], 0.)).xyz;
    out_uv = mix_4_values(vec4(in_uv[0], 0., 0.), vec4(in_uv[1], 0., 0.), vec4(in_uv[2], 0., 0.), vec4(in_uv[3], 0., 0.)).xy;
    out_color = mix_4_values(vec4(in_color[0], 0.), vec4(in_color[1], 0.), vec4(in_color[2], 0.), vec4(in_color[3], 0.)).xyz;
    out_tangent = mix_4_values(vec4(in_tangent[0], 0.), vec4(in_tangent[1], 0.), vec4(in_tangent[2], 0.), vec4(in_tangent[3], 0.)).xyz;
    out_bitangent = mix_4_values(vec4(in_bitangent[0], 0.), vec4(in_bitangent[1], 0.), vec4(in_bitangent[2], 0.), vec4(in_bitangent[3], 0.)).xyz;

    int octave = 4 - int(clamp(length(frame.camera.position - p.xyz) / 100., 0., 3.));
    vec3 mouvement = vec3(0.0, 0.0, 0.0);
    float j = 0.0;
    for (int i = 0; i < octave; i++) {
        vec4 waves = texture(waves[i], out_uv);
        mouvement += waves.xyz;
        j = waves.w;
    }
    out_position *= mouvement;

    out_color = mix(vec3(0., 0., 1.0), vec3(1.0, 1.0, 1.0), clamp(k * (m - j), 0.0, 1.0));

    gl_Position = frame.camera.view_proj * p;
}