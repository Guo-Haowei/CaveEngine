#pragma once
#include "IEditCmd.h"

#include "cave/core/ids/Entity.h"
#include "cave/core/ids/SceneId.h"

namespace cave {

class Scene;
class SceneRegistry;

class EditCmdBase : public IEditCmd {
public:
    EditCmdBase(SceneRegistry& scene_reg)
        : m_scene_reg(scene_reg) {
    }

    bool canCoalesceWith(const IEditCmd*) const override {
        return false;
    }

    void coalesceFrom(Owner<IEditCmd>) override {
        return;
    }

protected:
    Scene* resolveScene(SceneId scene_id) const;

private:
    SceneRegistry& m_scene_reg;
};

}  // namespace cave
