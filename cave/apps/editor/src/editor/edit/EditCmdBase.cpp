#include "EditCmdBase.h"

#include "engine/private/runtime/scene/SceneRegistry.h"

namespace cave {

Scene* EditCmdBase::resolveScene(SceneId scene_id) const {
    return m_scene_reg.resolve(scene_id);
}

}  // namespace cave
