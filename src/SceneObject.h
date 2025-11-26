#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <StaticMesh.h>
#include <Material.h>

#include <memory>

#include <glm/matrix.hpp>

#include "PassType.h"

namespace OM3D {

class SceneObject {

    public:
        SceneObject(std::shared_ptr<StaticMesh> mesh = nullptr, std::shared_ptr<Material> material = nullptr);

        void render(PassType pass_type=PassType::MAIN) const;

        const Material& material() const;

        void set_transform(const glm::mat4& tr);
        const glm::mat4& transform() const;
        BoundingSphere get_bounding_sphere() const {
            return _mesh->get_bounding_sphere();
        };
        bool collide(const Frustum& cam_frustum, const glm::vec3& cam_position) const {
            return _mesh->collide(cam_frustum, cam_position);
        }

    private:
        glm::mat4 _transform = glm::mat4(1.0f);

        std::shared_ptr<StaticMesh> _mesh;
        std::shared_ptr<Material> _material;
};

}

#endif // SCENEOBJECT_H
