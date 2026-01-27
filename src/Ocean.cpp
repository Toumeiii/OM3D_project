//
// Created by remi on 24/01/2026.
//

#include "Ocean.h"

#include "Scene.h"

namespace OM3D {
    Ocean::Ocean() : _program(Program::from_file("ocean.comp")) {
        const auto material = std::make_shared<Material>(Material::textured_pbr_material());
        if(auto [is_ok, value] = TextureData::from_file(std::string(data_path) + "ocean_texture.png"); is_ok) {
            const auto ocean_texture = std::make_shared<Texture>(value);
            material->set_texture(0u, ocean_texture);
        }
        material->set_program(Program::from_files(
                "ocean.frag",
                "ocean.tese",
                "ocean.tesc",
                "ocean.vert",
                4
            ));

        material->set_stored_uniform(HASH("alpha_cutoff"), 0.f);
        material->set_stored_uniform(HASH("base_color_factor"), glm::vec3(1., 1., .1));
        material->set_stored_uniform(HASH("metal_rough_factor"), glm::vec2(1., 1.));
        material->set_stored_uniform(HASH("emissive_factor"), glm::vec3(1., 1., 1.));
        _material = material;
    }

    void Ocean::set_iteration(const size_t iteration) {
        if (_iteration != iteration) {
            _iteration = iteration;
            _update_ocean = true;
        }
    }

    std::shared_ptr<std::vector<SceneObject>> Ocean::get_ocean(
            const Camera &camera,
            const float y_level,
            const float min_size,
            const float tesselation_level) const {
        const float y_dist = 1.f + glm::abs(camera.position().y - y_level);
        for (auto &obj : *_result) {
            obj.set_transform({
                min_size * y_dist, 0., 0., 0.,
                0., y_dist, 0., 0.,
                0., 0., min_size * y_dist, 0.,
                camera.position().x, y_level, camera.position().z, 1.,
            });
            obj.material().set_uniform(HASH("tesselation_level"), tesselation_level);
        }
        return _result;
    }

    std::shared_ptr<std::vector<SceneObject>> Ocean::get_ocean(
            const Camera &camera,
            const float y_level,
            const float min_size,
            const float tesselation_level) {
        if (_update_ocean) {
            _update_ocean = false;
            compute_ocean();
        }
        return static_cast<const Ocean*>(this)->get_ocean(camera, y_level, min_size, tesselation_level);
    }

    void Ocean::compute_ocean() {
        if (_iteration < 1)
            _result = std::make_shared<std::vector<SceneObject>>();

        const size_t count = (_iteration * 12 - 8);
        const TypedBuffer<glm::vec4> ocean(nullptr, count * 4);


        {
            ocean.bind(BufferUsage::Storage, 0);

            _program->bind();

            glDispatchCompute(1, _iteration, 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
        }

        const auto data = ocean.get_data();
        auto result = std::vector<SceneObject>(count);

        for (size_t i = 0; i < count; ++i) {
            const auto d = &data[i * 4];
            result[i] = SceneObject(
                std::make_shared<StaticMesh>(
                    MeshData{
                        std::vector{
                            Vertex {
                                d[0],
                                {0., 1., 0.},
                                {0., 0.},
                                {1.0f, 0.0f, 0.0f, 0.0f},
                                {1., 1., 1.},
                            },
                            Vertex {
                                d[1],
                                {0., 1., 0.},
                                {0., 1.},
                                {1.0f, 0.0f, 0.0f, 0.0f},
                                {1., 1., 1.},
                            },
                            Vertex {
                                d[2],
                                {0., 1., 0.,},
                                {1., 1.},
                                {1.0f, 0.0f, 0.0f, 0.0f},
                                {1., 1., 1.},
                            },
                            Vertex {
                                d[3],
                                {0., 1., 0.,},
                                {1., 0.},
                                {1.0f, 0.0f, 0.0f, 0.0f},
                                {1., 1., 1.},
                            },
                        },
                        { 0, 1, 2, 3 },
                    }
                ),
                _material
            );
            result[i].set_transform({
                1, 0., 0., 0.,
                0., 1., 0., 0.,
                0., 0., 1., 0.,
                0, 0., 0, 1.,
            });
        }

        *_result = std::move(result);
    }
}
