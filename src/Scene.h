#ifndef SCENE_H
#define SCENE_H

#include <SceneObject.h>
#include <PointLight.h>
#include <Camera.h>

#include <vector>
#include <memory>

#include "BufferMapping.h"
#include "BufferMapping.h"
#include "PassType.h"
#include "shader_structs.h"

namespace OM3D {

class Scene : NonMovable {

    public:
        Scene();

        static Result<std::unique_ptr<Scene>> from_gltf(const std::string& file_name);

        void render(PassType pass_type=PassType::MAIN) const;

        void add_object(SceneObject obj);
        void add_light(PointLight obj);
        void add_sphere(const std::shared_ptr<SceneObject> &obj);

        Span<const SceneObject> objects() const;
        Span<const PointLight> point_lights() const;

        Camera& camera();
        const Camera& camera() const;

        void set_envmap(std::shared_ptr<Texture> env);
        void set_ibl_intensity(float intensity);
        void set_sun(float altitude, float azimuth, glm::vec3 color = glm::vec3(1.0f));
        void set_frame_buffer(TypedBuffer<shader::FrameData> &buffer) const;

        Camera get_sun_camera(std::vector<const SceneObject*> *visible_objects = nullptr) const;

        void bind_envmap(int index = 4) const;

    private:
        std::vector<SceneObject> _objects;
        std::vector<PointLight> _point_lights;
        std::shared_ptr<SceneObject> _sphere;

        glm::vec3 _sun_direction = glm::vec3(0.2f, 1.0f, 0.1f);
        glm::vec3 _sun_color = glm::vec3(1.0f);

        std::shared_ptr<Texture> _envmap;
        float _ibl_intensity = 1.0f;
        Material _sky_material;

        Camera _camera;

        void set_light(TypedBuffer<shader::PointLight> &light_buffer) const;
        void set_frame_buffer_shadow(TypedBuffer<shader::FrameData> &buffer, const Camera &sun_camera) const;

        std::pair<std::vector<const SceneObject*>, std::vector<const SceneObject*>> get_opaque_transparent(const Camera &camera) const;

        void render_sky() const;
        void render_main(PassType pass_type) const;
        void render_deferred(PassType pass_type) const;
        void render_depth(PassType pass_type) const;
        void render_shadow(PassType pass_type) const;
        void render_sun_ibl(PassType pass_type) const;
        void render_point_lights(PassType pass_type) const;
        void render_alpha_lights(PassType pass_type) const;
};

}

#endif // SCENE_H
