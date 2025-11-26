#include "SceneObject.h"

#include "PassType.h"

namespace OM3D {

SceneObject::SceneObject(std::shared_ptr<StaticMesh> mesh, std::shared_ptr<Material> material) :
    _mesh(std::move(mesh)),
    _material(std::move(material)) {
}

void SceneObject::render(const PassType pass_type) const {
    if(!_material || !_mesh) {
        return;
    }
    _material->set_depth_test_mode(pass_type == PassType::DEPTH ? DepthTestMode::Equal : DepthTestMode::Standard);
    _material->set_uniform(HASH("model"), transform());
    _material->bind(pass_type);
    _mesh->draw();
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
