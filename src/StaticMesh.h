#ifndef STATICMESH_H
#define STATICMESH_H

#include <TypedBuffer.h>
#include <Vertex.h>

#include <vector>

#include "BoundingSphere.h"
#include "Camera.h"
#include "Program.h"
#include "glad/gl.h"

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

        void draw(ProgramType program_type = DEFAULT, GLint patch_size = 4) const;
        bool collide(const Camera& camera, const glm::mat4 &transform) const;
        BoundingSphere get_bounding_sphere() const;

    private:
        BoundingSphere _bounding_sphere;
        TypedBuffer<Vertex> _vertex_buffer;
        TypedBuffer<u32> _index_buffer;
};

}

#endif // STATICMESH_H
