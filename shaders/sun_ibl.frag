#version 450

#include "utils.glsl"
#include "lighting.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 in_uv;

layout(binding = 0) uniform sampler2D in_depth;
layout(binding = 1) uniform sampler2D in_albedo_roughness;
layout(binding = 2) uniform sampler2D in_normal_metalness;

layout(binding = 3) uniform samplerCube in_envmap;
layout(binding = 4) uniform sampler2D brdf_lut;
layout(binding = 5) uniform sampler2DShadow shadow_map;

layout(binding = 0) uniform Data {
    FrameData frame;
};

void main() {
    const ivec2 coord = ivec2(gl_FragCoord.xy);

    const float depth = texelFetch(in_depth, coord, 0).x;

    const vec3 base_color = texelFetch(in_albedo_roughness, coord, 0).xyz;
    const vec3 normal = texelFetch(in_normal_metalness, coord, 0).xyz;

    const float roughness = texelFetch(in_albedo_roughness, coord, 0).w;
    const float metallic = texelFetch(in_normal_metalness, coord, 0).w;

    const vec3 position = unproject(in_uv, depth, frame.camera.inv_view_proj);
    const vec3 to_view = (frame.camera.position - position);
    const vec3 view_dir = normalize(to_view);

    vec3 acc = eval_ibl(in_envmap, brdf_lut, normal, view_dir, base_color, metallic, roughness) * frame.ibl_intensity;
    {
        float shadow = get_shadow(shadow_map, frame.sun_view_proj, position);
        if (shadow > 0.) {
            acc += shadow * frame.sun_color * eval_brdf(normal, view_dir, frame.sun_dir, base_color, metallic, roughness);
        }
    }

    out_color = vec4(acc, 1);
}