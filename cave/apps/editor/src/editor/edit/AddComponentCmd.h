#pragma once
#include "EditCmdBase.h"

#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

class AddComponentCmd : public EditCmdBase {
public:
    AddComponentCmd(SceneRegistry& scene_reg,
                    ecs::Entity ent,
                    ComponentId cid);

    const char* Label() const override { return "AddComponentCmd"; }

    bool Do(IDocument& doc) override;
    bool Undo(IDocument& doc) override;

private:
    ComponentId m_cid;
};

}  // namespace cave
