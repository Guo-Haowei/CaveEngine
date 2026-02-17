#pragma once
#include "EditCmdBase.h"

#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

class AddComponentCmd : public EditCmdBase {
public:
    AddComponentCmd(SceneRegistry& p_scene_reg,
                    ecs::Entity p_ent,
                    ComponentId p_cid);

    const char* Label() const override { return "AddComponentCmd"; }

    bool Do(IDocument& p_doc) override;
    bool Undo(IDocument& p_doc) override;

private:
    ComponentId m_cid;
};

}  // namespace cave
