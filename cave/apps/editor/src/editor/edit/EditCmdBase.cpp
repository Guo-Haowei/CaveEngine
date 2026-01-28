#include "EditCmdBase.h"

#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/scene/ISceneRegistry.h"

namespace cave {

EditCmdCtx::EditCmdCtx(IApplication& p_app)
    : m_scene_reg(*p_app.GetSceneRegistry()) {
}

Scene* EditCmdCtx::ResolveScene(SceneId p_scene_id) {
    return m_scene_reg.Resolve(p_scene_id);
}

}  // namespace cave
