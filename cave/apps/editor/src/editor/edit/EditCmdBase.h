#pragma once
#include "IEditCmd.h"

#include "cave/core/ids/Entity.h"
#include "cave/core/ids/SceneId.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave {

class IApplication;
class ISceneRegistry;

class EditCmdBase : public IEditCmd {
public:
    EditCmdBase(IApplication& p_app,
                ecs::Entity p_entity);

    bool CanCoalesceWith(const IEditCmd*) const override {
        return false;
    }

    void CoalesceFrom(std::unique_ptr<IEditCmd>) override {
        return;
    }

protected:
    Scene* ResolveScene(SceneId p_scene_id);
    ecs::Entity m_entity;

private:
    ISceneRegistry& m_scene_reg;
};

}  // namespace cave
