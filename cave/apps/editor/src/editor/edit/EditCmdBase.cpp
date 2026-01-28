#include "EditCmdBase.h"

#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/scene/ISceneRegistry.h"

namespace cave {

EditCmdBase::EditCmdBase(IApplication& p_app,
                         ecs::Entity p_entity)
    : m_scene_reg(*p_app.GetSceneRegistry())
    , m_entity(p_entity) {
}

Scene* EditCmdBase::ResolveScene(SceneId p_scene_id) {
    return m_scene_reg.Resolve(p_scene_id);
}

}  // namespace cave
