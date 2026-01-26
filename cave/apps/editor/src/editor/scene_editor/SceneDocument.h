#pragma once
#include "cave/runtime/scene/SceneId.h"

#include "engine/private/ecs/entity.h"
#include "engine/private/math/geomath.h"

#include "editor/document/document.h"

namespace cave {

class ISceneManager;
class Scene;

class SceneDocument : public Document {
public:
    SceneDocument(const Guid& p_guid, ISceneManager& p_scene_manager);

    void RequestMove(ecs::Entity p_entity,
                     const Matrix4x4f& p_before,
                     const Matrix4x4f& p_after,
                     bool p_execute);

    bool Save() override;

    SceneId GetSceneId() const { return m_scene_id; }

private:
    ISceneManager& m_scene_manager;
    SceneId m_scene_id;                    // this is the runtime scene
    std::shared_ptr<Scene> m_asset_scene;  // this is the asset scene
};

}  // namespace cave
