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
        std::shared_ptr<std::vector<SceneObject>> _result = std::make_shared<std::vector<SceneObject>>();
        size_t _iteration = 5;
        bool _update_ocean = true;

        void compute_ocean();

    public:

        Ocean();

        void set_iteration(size_t iteration);

        [[nodiscard]] std::shared_ptr<std::vector<SceneObject>> get_ocean(const Camera &camera, float y_level, float min_size, float tesselation_level) const;
        [[nodiscard]] std::shared_ptr<std::vector<SceneObject>> get_ocean(const Camera &camera, float y_level, float min_size, float tesselation_level);
    };
}

#endif //OM3D_SQUARE_HH
