//
// Created by remi on 10/12/2025.
//

#include "BoundingSphere.h"

#include "Camera.h"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "glm/vec4.hpp"

bool BoundingSphere::collide(const OM3D::Camera& camera, const glm::mat4 &transform) const {
    if (radius <= 0.f)
        return false;

    const float sx = glm::length(glm::vec3(transform[0]));
    const float sy = glm::length(glm::vec3(transform[1]));
    const float sz = glm::length(glm::vec3(transform[2]));

    const float world_radius = radius * std::max(std::max(sx, sy), sz);
    const auto world_origin = glm::vec3(transform * glm::vec4(origin, 1.));

    auto in_plane = [world_radius](const glm::vec3& n, const glm::vec3& p) {
        return glm::dot(glm::normalize(n), p) > - world_radius;
    };

    auto [near_normal, top_normal, bottom_normal, right_normal, left_normal] = camera.build_frustum();


    if (camera.is_orthographic()) {

        // Get camera matrices
        const glm::mat4 view = camera.view_matrix();
        const glm::mat4 proj = camera.projection_matrix();
        const glm::mat4 invView = glm::inverse(view);

        const glm::vec3 camPos   = glm::vec3(invView[3]);
        const glm::vec3 camRight = glm::normalize(glm::vec3(invView[0]));
        const glm::vec3 camUp    = glm::normalize(glm::vec3(invView[1]));
        const glm::vec3 camFwd   = glm::normalize(glm::vec3(invView[2]));

        const float left   = -(proj[3][0] + 1.0f) / proj[0][0];
        const float right  =  (1.0f - proj[3][0]) / proj[0][0];
        const float bottom = -(proj[3][1] + 1.0f) / proj[1][1];
        const float top    =  (1.0f - proj[3][1]) / proj[1][1];
        const float nearZ  =  (proj[3][2] + 1.0f) / proj[2][2];

        const glm::vec3 c_left   = world_origin - (camPos + camRight * left);
        const glm::vec3 c_right  = world_origin - (camPos + camRight * right);
        const glm::vec3 c_top    = world_origin - (camPos + camUp    * top);
        const glm::vec3 c_bottom = world_origin - (camPos + camUp    * bottom);
        const glm::vec3 c_near   = world_origin - (camPos + camFwd   * nearZ);

        return in_plane(bottom_normal, c_bottom)
            && in_plane(left_normal,   c_left)
            && in_plane(right_normal,  c_right)
            && in_plane(top_normal,    c_top)
            && in_plane(near_normal,   c_near);
    } else {
        const auto c = world_origin - camera.position();
        return in_plane(bottom_normal, c)
            && in_plane(left_normal, c)
            && in_plane(right_normal, c)
            && in_plane(top_normal, c)
            && in_plane(near_normal, c);
    }
}
