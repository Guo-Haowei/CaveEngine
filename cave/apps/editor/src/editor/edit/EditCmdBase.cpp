#include "EditCmdBase.h"

#include "engine/private/runtime/scene/SceneRegistry.h"

namespace cave {

EditCmdBase::EditCmdBase(SceneRegistry& p_scene_reg, ecs::Entity p_ent)
    : scene_reg_(p_scene_reg)
    , ent_(p_ent) {
}

Scene* EditCmdBase::resolveScene(SceneId p_scene_id) const {
    return scene_reg_.resolve(p_scene_id);
}

}  // namespace cave
