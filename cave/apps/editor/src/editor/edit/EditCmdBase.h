#pragma once
#include "IEditCmd.h"

#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/scene/SceneId.h"

#include "engine/private/runtime/scene/Scene.h"

#include "editor/document/DocumentTypes.h"

namespace cave {

class IApplication;
class ISceneRegistry;

// @TODO: refactor this
class EditCmdCtx {
    ISceneRegistry& m_scene_reg;

public:
    ecs::Entity entity{};

    EditCmdCtx(IApplication& p_app);

    Scene* ResolveScene(SceneId p_scene_id);
};

}  // namespace cave
