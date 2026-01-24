//
// Created by remi on 21/01/2026.
//

#ifndef OM3D_SQUARE_HH
#define OM3D_SQUARE_HH

#include "SceneObject.h"

namespace OM3D {
    class Ocean {
    private:
        std::shared_ptr<Material> _material;
        const Vertex _model_vertices[4] = {
            Vertex{
                {0., 0., 0.},
                {0., 1., 0.},
                {0., 0.},
                {1.0f, 0.0f, 0.0f, 0.0f},
                {1., 1., 1.},
            },
            Vertex{
                {0., 0., 1.},
                {0., 1., 0.},
                {0., 1.},
                {1.0f, 0.0f, 0.0f, 0.0f},
                {1., 1., 1.},
            },
            Vertex{
                {1., 0., 1.,},
                {0., 1., 0.,},
                {1., 1.},
                {1.0f, 0.0f, 0.0f, 0.0f},
                {1., 1., 1.},
            },
            Vertex{
                {1., 0., 0.,},
                {0., 1., 0.,},
                {1., 0.},
                {1.0f, 0.0f, 0.0f, 0.0f},
                {1., 1., 1.},
            },
        };
        std::shared_ptr<Program> _program;

    public:
        float y_level = 0.f;
        float min_size = .1f;
        size_t iteration = 5;

        Ocean();
        [[nodiscard]] std::vector<SceneObject> get_ocean(const Camera &camera, float tesselation_level) const;
    };
}

#endif //OM3D_SQUARE_HH
