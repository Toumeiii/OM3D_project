//
// Created by remi on 25/11/2025.
//

#ifndef OM3D_BOUNDINGSPHERE_HH
#define OM3D_BOUNDINGSPHERE_HH

#include "glm/vec3.hpp"

#include "Camera.h"

struct BoundingSphere {
    glm::vec3 origin;
    float radius;

    bool collide(const OM3D::Camera& camera, const glm::mat4 &transform) const;
};

#endif //OM3D_BOUNDINGSPHERE_HH