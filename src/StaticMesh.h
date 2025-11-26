#ifndef STATICMESH_H
#define STATICMESH_H

#include <TypedBuffer.h>
#include <Vertex.h>

#include <vector>

#include "BoundingSphere.h"
#include "Camera.h"

namespace OM3D {

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
};

class StaticMesh : NonCopyable {

    public:
        StaticMesh() = default;
        StaticMesh(StaticMesh&&) = default;
        StaticMesh& operator=(StaticMesh&&) = default;

        StaticMesh(const MeshData& data);

        void draw() const;
        bool collide(const Frustum& cam_frustum, const glm::vec3& cam_position) const;
        BoundingSphere get_bounding_sphere() const;

    private:
        BoundingSphere _bounding_sphere;
        TypedBuffer<Vertex> _vertex_buffer;
        TypedBuffer<u32> _index_buffer;
};

}

#endif // STATICMESH_H
