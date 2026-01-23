//
// Created by remi on 21/01/2026.
//

#ifndef OM3D_SQUARE_HH
#define OM3D_SQUARE_HH
#include "Vertex.h"

namespace OM3D {
    inline SceneObject get_ocean() {

        auto material = std::make_shared<Material>(Material::textured_pbr_material());
        if(auto res = TextureData::from_file(std::string(data_path) + "ocean_texture.jpg"); res.is_ok) {
            auto ocean_texture = std::make_shared<Texture>(res.value);
            material->set_texture(0u, ocean_texture);
            material->set_texture(2u, default_metal_rough_texture());
        }

        auto ocean = SceneObject(
            std::make_shared<StaticMesh>(
                MeshData{
                    std::vector{
                        Vertex {
                            {0., 0., 0.},
                            {0., 1., 0.},
                            {0., 0.},
                            {1.0f, 0.0f, 0.0f, 0.0f},
                            {1., 1., 1.},
                        },
                        Vertex {
                            {1., 0., 1.,},
                            {0., 1., 0.,},
                            {1., 1.},
                            {1.0f, 0.0f, 0.0f, 0.0f},
                            {1., 1., 1.},
                        },
                        Vertex {
                            {1., 0., 0.,},
                            {0., 1., 0.,},
                            {1., 0.},
                            {1.0f, 0.0f, 0.0f, 0.0f},
                            {1., 1., 1.},
                        },Vertex {
                            {0., 0., 0.},
                            {0., 1., 0.},
                            {0., 0.},
                            {1.0f, 0.0f, 0.0f, 0.0f},
                            {1., 1., 1.},
                        },
                        Vertex {
                            {0., 0., 1.,},
                            {0., 1., 0.,},
                            {0., 1.},
                            {1.0f, 0.0f, 0.0f, 0.0f},
                            {1., 1., 1.},
                        },
                        Vertex {
                            {1., 0., 1.,},
                            {0., 1., 0.,},
                            {1., 1.},
                            {1.0f, 0.0f, 0.0f, 0.0f},
                            {1., 1., 1.},
                        },
                    },
                    { 0, 1, 2, 3, 4, 5 },
                }
            ),
            std::move(material)
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
