//
// Created by remi on 21/01/2026.
//

#ifndef OM3D_SQUARE_HH
#define OM3D_SQUARE_HH
#include "Vertex.h"

namespace OM3D {
    inline SceneObject get_ocean() {
        auto ocean = SceneObject(
            std::make_shared<StaticMesh>(
                MeshData{
                    std::vector{
                        Vertex {
                            {0., 0., 0.},
                            {0., 1., 0.},
                            {0., 0.},
                            {0.0f, 0.0f, 0.0f, 0.0f},
                            {0., 0., 1.},
                        },
                        Vertex {
                            {1., 0., 1.,},
                            {0., 1., 0.,},
                            {1., 1.},
                            {0.0f, 0.0f, 0.0f, 0.0f},
                            {0., 0., 1.},
                        },
                        Vertex {
                            {1., 0., 0.,},
                            {0., 1., 0.,},
                            {1., 0.},
                            {0.0f, 0.0f, 0.0f, 0.0f},
                            {0., 0., 1.},
                        },Vertex {
                            {0., 0., 0.},
                            {0., 1., 0.},
                            {0., 0.},
                            {0.0f, 0.0f, 0.0f, 0.0f},
                            {0., 0., 1.},
                        },
                        Vertex {
                            {0., 0., 1.,},
                            {0., 1., 0.,},
                            {0., 1.},
                            {0.0f, 0.0f, 0.0f, 0.0f},
                            {0., 0., 1.},
                        },
                        Vertex {
                            {1., 0., 1.,},
                            {0., 1., 0.,},
                            {1., 1.},
                            {0.0f, 0.0f, 0.0f, 0.0f},
                            {0., 0., 1.},
                        },
                    },
                    { 0, 1, 2, 3, 4, 5 },
                }
            ),
            std::make_shared<Material>(Material::textured_pbr_material())
        );
        ocean.set_transform({
            100., 0., 0., 0.,
            0., 1., 0., 0.,
            0., 0., 100., 0.,
            -50., 0., -50., 1.,
        });
        return ocean;
    }
}

#endif //OM3D_SQUARE_HH
