#version 450

layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D depth;
layout(binding = 1) uniform sampler2D albedo_roughness;
layout(binding = 2) uniform sampler2D normal_metalness;

uniform uint state;

void main() {
    const ivec2 coord = ivec2(gl_FragCoord.xy);

    if (state == 1) {       // depth
        out_color = (texelFetch(depth, coord, 0).z > 0.) ? vec4(1.) : vec4(0.);
    }
    else if (state == 2) {  // albedo
        out_color = vec4(texelFetch(depth, coord, 0).xyz, 1.);
    }
    else if (state == 3) {  // normal
        out_color = vec4(texelFetch(depth, coord, 0).xyz, 1.);
    }
    else if (state == 4) {  // roughness
        out_color = vec4(texelFetch(depth, coord, 0).www, 1.);
    }
    else if (state == 5) {  // metalness
        out_color = vec4(texelFetch(depth, coord, 0).www, 1.);
    }
}