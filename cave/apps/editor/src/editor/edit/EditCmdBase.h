#pragma once
#include "IEditCmd.h"

#include "cave/core/ids/SceneId.h"
#include "cave/runtime/ecs/Entity.h"

namespace cave {

class Scene;
class SceneRegistry;

class EditCmdBase : public IEditCmd {
public:
    EditCmdBase(SceneRegistry& scene_reg, ecs::Entity ent);

    bool canCoalesceWith(const IEditCmd*) const override {
        return false;
    }

    void coalesceFrom(std::unique_ptr<IEditCmd>) override {
        return;
    }

protected:
    Scene* resolveScene(SceneId scene_id) const;
    ecs::Entity ent_;

private:
    SceneRegistry& scene_reg_;
};

}  // namespace cave
