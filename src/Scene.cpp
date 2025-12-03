#include "Scene.h"

#include <TypedBuffer.h>

#include <shader_structs.h>
#include <vector>

#include "glad/gl.h"
#include "glm/ext/quaternion_geometric.hpp"

namespace OM3D {

Scene::Scene() {
    _sky_material.set_program(Program::from_files("sky.frag", "screen.vert"));
    _sky_material.set_depth_test_mode(DepthTestMode::None);

    _envmap = std::make_shared<Texture>(Texture::empty_cubemap(4, ImageFormat::RGBA8_UNORM));
}

void Scene::add_object(SceneObject obj) {
    _objects.emplace_back(std::move(obj));
}

void Scene::add_light(PointLight obj) {
    _point_lights.emplace_back(std::move(obj));
}

Span<const SceneObject> Scene::objects() const {
    return _objects;
}

Span<const PointLight> Scene::point_lights() const {
    return _point_lights;
}

Camera& Scene::camera() {
    return _camera;
}

const Camera& Scene::camera() const {
    return _camera;
}

Camera Scene::get_sun_camera(std::vector<const SceneObject*> *visible_objects) const{
    bool to_delete = false;
    if (visible_objects == nullptr) {
        to_delete = true;
        visible_objects = new std::vector<const SceneObject*>();
        const auto cam_frustum = _camera.build_frustum();
        const auto cam_position= _camera.position();

        for(const SceneObject& obj : _objects) {
            if(obj.material().is_opaque() && obj.collide(cam_frustum, cam_position)) {
                visible_objects->emplace_back(&obj);
            }
        }
    }

    BoundingSphere bounding_sphere;

    if (visible_objects->empty()) {
        bounding_sphere.origin = glm::vec3(0);
        bounding_sphere.radius = 1.f;
    }
    else {
        glm::vec3 min = {};
        glm::vec3 max = {};

        {
            auto [origin, radius] = (*visible_objects)[0]->get_bounding_sphere();
            min = origin - radius;
            max = origin + radius;
        }

        for (auto object : *visible_objects) {
            auto [origin, radius] = object->get_bounding_sphere();
            min = {
                glm::min(min.x, origin.x - radius),
                glm::min(min.y, origin.y - radius),
                glm::min(min.z, origin.z - radius),
            };
            max = {
                glm::max(max.x, origin.x + radius),
                glm::max(max.y, origin.y + radius),
                glm::max(max.z, origin.z + radius),
            };
        }
        bounding_sphere.origin = (min + max) / 2.f;
        bounding_sphere.radius = glm::length(max - min) / 2.f;
    }
    if (to_delete) delete visible_objects;
    auto cam = Camera();

    cam.set_proj(Camera::orthographic(
        -bounding_sphere.radius,
        bounding_sphere.radius,
        -bounding_sphere.radius,
        bounding_sphere.radius,
        0,
        bounding_sphere.radius * 2
        ));

    cam.set_view(glm::lookAt(
        bounding_sphere.origin + glm::normalize(_sun_direction) * bounding_sphere.radius,
        bounding_sphere.origin,
        glm::vec3(0, 1, 0)
        ));
    return cam;
}

void Scene::set_envmap(std::shared_ptr<Texture> env) {
    _envmap = std::move(env);
}

void Scene::set_ibl_intensity(float intensity) {
    _ibl_intensity = intensity;
}

void Scene::set_sun(float altitude, float azimuth, glm::vec3 color) {
    // Convert from degrees to radians
    const float alt = glm::radians(altitude);
    const float azi = glm::radians(azimuth);
    // Convert from polar to cartesian
    _sun_direction = glm::vec3(sin(azi) * cos(alt), sin(alt), cos(azi) * cos(alt));
    _sun_color = color;
}

void Scene::render(const PassType pass_type) const {
    // Fill and bind frame data buffer
    TypedBuffer<shader::FrameData> buffer(nullptr, 1);
    if (pass_type != PassType::SHADOW) {
        auto mapping = buffer.map(AccessType::WriteOnly);
        mapping[0].camera.view_proj = _camera.view_proj_matrix();
        mapping[0].camera.inv_view_proj = glm::inverse(_camera.view_proj_matrix());
        mapping[0].camera.position = _camera.position();
        mapping[0].point_light_count = u32(_point_lights.size());
        mapping[0].sun_color = _sun_color;
        mapping[0].sun_dir = glm::normalize(_sun_direction);
        mapping[0].ibl_intensity = _ibl_intensity;
        mapping[0].sun_inv_view_proj = glm::inverse(get_sun_camera().view_proj_matrix());
        buffer.bind(BufferUsage::Uniform, 0);
    }

    if (pass_type == PassType::MAIN) {
        // Fill and bind lights buffer
        TypedBuffer<shader::PointLight> light_buffer(nullptr, std::max(_point_lights.size(), size_t(1)));
        {
            auto mapping = light_buffer.map(AccessType::WriteOnly);
            for(size_t i = 0; i != _point_lights.size(); ++i) {
                const auto& light = _point_lights[i];
                mapping[i] = {
                    light.position(),
                    light.radius(),
                    light.color(),
                    0.0f
                };
            }
        }
        light_buffer.bind(BufferUsage::Storage, 1);

        // Bind envmap
        DEBUG_ASSERT(_envmap && !_envmap->is_null());
        _envmap->bind(4);

        // Bind brdf lut needed for lighting to scene rendering shaders
        brdf_lut().bind(5);

        // Render the sky
        _sky_material.bind();
        _sky_material.set_uniform(HASH("intensity"), _ibl_intensity);
        draw_full_screen_triangle();
    }

    const auto cam_frustum = _camera.build_frustum();
    const auto cam_position= _camera.position();
    // Render every object
    if (pass_type != PassType::SHADOW) {
        // Opaque first
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Opaques");
        for(const SceneObject& obj : _objects) {
            if(obj.material().is_opaque()) {
                if (obj.collide(cam_frustum, cam_position)) obj.render(pass_type);
            }
        }
        glPopDebugGroup(); // Opaques

        // Transparent after
        if (pass_type == PassType::MAIN) {
            glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Transparents");
            for(const SceneObject& obj : _objects) {
                if(!obj.material().is_opaque()) {
                    if (obj.collide(cam_frustum, cam_position)) obj.render(pass_type);
                }
            }
            glPopDebugGroup(); // Transparents
        }
    }
    else {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Opaques");

        std::vector<const SceneObject*> visible_objects;

        for(const SceneObject& obj : _objects) {
            if(obj.material().is_opaque() && obj.collide(cam_frustum, cam_position)) {
                visible_objects.emplace_back(&obj);
            }
        }

        auto sun_camera = get_sun_camera(&visible_objects);

        auto mapping = buffer.map(AccessType::WriteOnly);
        mapping[0].camera.view_proj = sun_camera.projection_matrix();
        mapping[0].camera.inv_view_proj = glm::inverse(sun_camera.view_proj_matrix());
        mapping[0].camera.position = sun_camera.position();
        mapping[0].point_light_count = u32(_point_lights.size());
        mapping[0].sun_color = _sun_color;
        mapping[0].sun_dir = glm::normalize(_sun_direction);
        mapping[0].sun_inv_view_proj = glm::inverse(sun_camera.view_proj_matrix());
        buffer.bind(BufferUsage::Uniform, 0);

        for(const SceneObject *obj : visible_objects) {
            obj->render(PassType::DEPTH);
        }
        glPopDebugGroup(); // Opaques
    }

}

}
