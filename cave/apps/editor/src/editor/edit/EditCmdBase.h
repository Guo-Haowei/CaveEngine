#pragma once
#include "IEditCmd.h"

#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/scene/SceneId.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave {

class IApplication;
class ISceneRegistry;

class EditCmdBase : public IEditCmd {
public:
    EditCmdBase(IApplication& p_app,
                ecs::Entity p_entity);

protected:
    Scene* ResolveScene(SceneId p_scene_id);
    ecs::Entity m_entity;

private:
    ISceneRegistry& m_scene_reg;
};

}  // namespace cave
