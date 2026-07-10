#pragma once
#include "IEditCmd.h"

#include "cave/core/ids/Entity.h"
#include "cave/core/ids/SceneId.h"

namespace cave {

class Scene;
class SceneRegistry;

class EditCmdBase : public IEditCmd {
public:
    EditCmdBase(SceneRegistry& scene_reg, ecs::Entity ent);

    bool canCoalesceWith(const IEditCmd*) const override {
        return false;
    }

    void coalesceFrom(Owner<IEditCmd>) override {
        return;
    }

protected:
    Scene* resolveScene(SceneId scene_id) const;

    ecs::Entity m_ent;

private:
    SceneRegistry& m_scene_reg;
};

}  // namespace cave
