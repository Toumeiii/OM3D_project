#include "StaticMesh.h"

#include <glad/gl.h>

#include "glm/glm.hpp"
#include "glm/geometric.hpp"

namespace OM3D {
    extern bool audit_bindings_before_draw;

    StaticMesh::StaticMesh(const MeshData& data) :
        _vertex_buffer(data.vertices),
        _index_buffer(data.indices) {
        if (data.vertices.empty()) {
            _bounding_sphere.origin = glm::vec3(INFINITY);
            _bounding_sphere.radius = 0.f;
        }
        else {
            glm::vec3 min = data.vertices[0].position;
            glm::vec3 max = data.vertices[0].position;

            for (auto vertex : data.vertices) {
                min = {
                    glm::min(min.x, vertex.position.x),
                    glm::min(min.y, vertex.position.y),
                    glm::min(min.z, vertex.position.z),
                };
                max = {
                    glm::max(max.x, vertex.position.x),
                    glm::max(max.y, vertex.position.y),
                    glm::max(max.z, vertex.position.z),
                };
            }
            _bounding_sphere.origin = (min + max) / 2.f;
            _bounding_sphere.radius = glm::length(max - min) / 2.f;
        }
    }

    bool StaticMesh::collide(const Frustum& cam_frustum, const glm::vec3& cam_position) const
    {
        if (_bounding_sphere.radius <= 0.f)
            return false;
        const auto c = _bounding_sphere.origin - cam_position;
        auto in_plane = [&](const glm::vec3& n) {
            return glm::dot(glm::normalize(n), c) > -_bounding_sphere.radius;
        };

        return in_plane(cam_frustum._bottom_normal)
            && in_plane(cam_frustum._left_normal)
            && in_plane(cam_frustum._right_normal)
            && in_plane(cam_frustum._top_normal)
            && in_plane(cam_frustum._bottom_normal);
    }

    BoundingSphere StaticMesh::get_bounding_sphere() const {
        return _bounding_sphere;
    }

    void StaticMesh::draw() const {
    _vertex_buffer.bind(BufferUsage::Attribute);
    _index_buffer.bind(BufferUsage::Index);

    // Vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), nullptr);
    // Vertex normal
    glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(3 * sizeof(float)));
    // Vertex uv
    glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(6 * sizeof(float)));
    // Tangent / bitangent sign
    glVertexAttribPointer(3, 4, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(8 * sizeof(float)));
    // Vertex color
    glVertexAttribPointer(4, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(12 * sizeof(float)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    if(audit_bindings_before_draw) {
        audit_bindings();
    }

    glDrawElements(GL_TRIANGLES, int(_index_buffer.element_count()), GL_UNSIGNED_INT, nullptr);
}

}
