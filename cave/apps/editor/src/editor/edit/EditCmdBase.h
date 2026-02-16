#pragma once
#include "IEditCmd.h"

#include "cave/core/ids/SceneId.h"
#include "cave/runtime/ecs/Entity.h"

namespace cave {

class IApplication;
class SceneRegistry;

class Scene;

class EditCmdBase : public IEditCmd {
public:
    EditCmdBase(IApplication& p_app, ecs::Entity p_ent);

    bool CanCoalesceWith(const IEditCmd*) const override {
        return false;
    }

    void CoalesceFrom(std::unique_ptr<IEditCmd>) override {
        return;
    }

protected:
    Scene* ResolveScene(SceneId p_scene_id) const;
    ecs::Entity m_ent;

private:
    SceneRegistry& m_scene_reg;
};

}  // namespace cave
