#pragma once
#include "EditCmdBase.h"

#include "editor/document/SceneDocument.h"

namespace cave {

enum class EntityType : uint8_t;

class AddObjectCmd : public EditCmdBase {
public:
    AddObjectCmd(IApplication& p_app,
                 ecs::Entity p_entity,
                 EntityType p_type)
        : EditCmdBase(p_app, p_entity)
        , m_type(p_type) {}

    const char* Label() const override {
        return "AddObjectCmd";
    }

    bool Do(IDocument& p_doc) override;

    bool Undo(IDocument&) override;

protected:
    EntityType m_type;
    ecs::Entity m_created;
};

}  // namespace cave
