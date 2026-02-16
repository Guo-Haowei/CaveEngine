#pragma once
#include "EditCmdBase.h"

#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

class AddComponentCmd : public EditCmdBase {
public:
    AddComponentCmd(IApplication& p_app,
                    ecs::Entity p_ent,
                    BuiltinComponentId p_cid);

    const char* Label() const override { return "AddComponentCmd"; }

    bool Do(IDocument& p_doc) override;
    bool Undo(IDocument& p_doc) override;

private:
    BuiltinComponentId m_cid;
};

}  // namespace cave
