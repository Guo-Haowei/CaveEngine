#include "EditCmdBase.h"

#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/scene/SceneRegistry.h"

namespace cave {

EditCmdBase::EditCmdBase(IApplication& p_app,
                         ecs::Entity p_ent)
    : m_scene_reg(*p_app.GetSceneRegistry())
    , m_entity(p_ent) {
}

Scene* EditCmdBase::ResolveScene(SceneId p_scene_id) const {
    return m_scene_reg.Resolve(p_scene_id);
}

}  // namespace cave
