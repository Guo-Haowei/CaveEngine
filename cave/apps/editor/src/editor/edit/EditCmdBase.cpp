#include "EditCmdBase.h"

#include "engine/private/runtime/scene/SceneRegistry.h"

namespace cave {

EditCmdBase::EditCmdBase(SceneRegistry& p_scene_reg, ecs::Entity p_ent)
    : m_scene_reg(p_scene_reg)
    , m_ent(p_ent) {
}

Scene* EditCmdBase::resolveScene(SceneId p_scene_id) const {
    return m_scene_reg.resolve(p_scene_id);
}

}  // namespace cave
