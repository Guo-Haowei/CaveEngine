#pragma once
#include "EditCmdBase.h"

#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

class AddComponentCmd : public EditCmdBase {
public:
    AddComponentCmd(SceneRegistry& scene_reg,
                    ecs::Entity ent,
                    ComponentId cid);

    const char* label() const override { return "AddComponentCmd"; }

    bool apply(IDocument& doc) override;
    bool undo(IDocument& doc) override;

private:
    ecs::Entity m_ent;
    ComponentId m_cid;
};

}  // namespace cave
