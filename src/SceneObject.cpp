#include "SceneObject.h"

#include "PassType.h"
#include "glm/fwd.hpp"

namespace OM3D {

SceneObject::SceneObject(std::shared_ptr<StaticMesh> mesh, std::shared_ptr<Material> material) :
    _mesh(std::move(mesh)),
    _material(std::move(material)) {
}

void SceneObject::render(const PassType pass_type, const size_t i) const {
    if(!_material || !_mesh) {
        return;
    }
    switch(pass_type) {
        case PassType::MAIN:
            _material->set_depth_test_mode(DepthTestMode::Standard);
            _material->set_blend_mode(BlendMode::None);
            break;
        case PassType::DEPTH:
            _material->set_depth_test_mode(DepthTestMode::Standard);
            break;
        case PassType::POINT_LIGHT:
            _material->set_depth_test_mode(DepthTestMode::None);
            _material->set_blend_mode(BlendMode::PointLights);
            _material->set_stored_uniform(HASH("index"), static_cast<glm::u32>(i));
            break;
        case PassType::ALPHA_LIGHT:
            _material->set_depth_test_mode(DepthTestMode::None);
            break;
        default:
            _material->set_depth_test_mode(DepthTestMode::Equal);
            break;
    }
    _material->set_stored_uniform(HASH("model"), transform());
    _material->set_stored_uniform(HASH("is_main"), static_cast<u32>(pass_type == PassType::MAIN));
    _material->bind(pass_type);
    _mesh->draw(_material->get_program()->get_program_type(), _material->get_program()->get_patch_size());
}

const Material& SceneObject::material() const {
    DEBUG_ASSERT(_material);
    return *_material;
}

void SceneObject::set_transform(const glm::mat4& tr) {
    _transform = tr;
}

const glm::mat4& SceneObject::transform() const {
    return _transform;
}

}
