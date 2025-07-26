#pragma once
#include "engine/ecs/entity.h"
#include "engine/math/geomath.h"
#include "editor/document/document.h"

namespace cave {

class Scene;

class SceneDocument : public Document {
public:
    SceneDocument(const Guid& p_guid)
        : Document(p_guid) {
        m_scene = m_handle.Wait<Scene>();
    }

    void RequestMove(ecs::Entity p_entity,
                     const Matrix4x4f& p_before,
                     const Matrix4x4f& p_after,
                     bool p_execute);

private:
    std::shared_ptr<Scene> m_scene;

    friend class SceneEditor;
};

}  // namespace cave
