#version 450

#include "utils.glsl"
#include "lighting.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 3) in vec3 in_position;

layout(binding = 4) uniform samplerCube in_envmap;
layout(binding = 5) uniform sampler2D brdf_lut;
layout(binding = 6) uniform sampler2DShadow shadow_map;

layout(binding = 7) uniform sampler2D in_depth;
layout(binding = 8) uniform sampler2D in_albedo_roughness;
layout(binding = 9) uniform sampler2D in_normal_metalness;

uniform uint index;

layout(binding = 0) uniform Data {
    FrameData frame;
};

layout(binding = 1) buffer PointLights {
    PointLight point_lights[];
};

bool is_point_illuminate(PointLight light, vec3 position) {
    return length(light.position - position) <= light.radius;
}

void main() {
    const ivec2 coord = ivec2(gl_FragCoord.xy);

    const float depth = texelFetch(in_depth, coord, 0).x;

    vec3 acc = vec3(0.);

    const vec3 base_color = texelFetch(in_albedo_roughness, coord, 0).xyz;
    const vec3 normal = texelFetch(in_normal_metalness, coord, 0).xyz * 2. - 1.;

    const float roughness = texelFetch(in_albedo_roughness, coord, 0).w;
    const float metallic = texelFetch(in_normal_metalness, coord, 0).w;

    vec2 uv = vec2(gl_FragCoord.xy) / vec2(textureSize(in_albedo_roughness, 0));
    const vec3 position = unproject(uv, depth, frame.camera.inv_view_proj);

    PointLight light = point_lights[index];
    if (is_point_illuminate(light, position)) {
        const vec3 to_view = (frame.camera.position - position);
        const vec3 view_dir = normalize(to_view);

        const vec3 to_light = (light.position - position);
        const float dist = length(to_light);
        const vec3 light_vec = to_light / dist;

        const float att = attenuation(dist, light.radius);
        if (att > 0.0f) {
            acc += eval_brdf(normal, view_dir, light_vec, base_color, metallic, roughness) * att * light.color;
        }
    }

    out_color = vec4(acc, 0);
}