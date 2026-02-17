#pragma once
#include "EditCmdBase.h"

#include "editor/document/SceneDocument.h"

namespace cave {

enum class EntityType : uint8_t;

class AddObjectCmd : public EditCmdBase {
public:
    AddObjectCmd(SceneRegistry& p_scene_reg,
                 ecs::Entity p_ent,
                 EntityType p_type)
        : EditCmdBase(p_scene_reg, p_ent)
        , m_type(p_type) {}

    const char* Label() const override { return "AddObjectCmd"; }

    bool Do(IDocument& p_doc) override;

    bool Undo(IDocument&) override;

protected:
    EntityType m_type;
    ecs::Entity m_created;
};

class DeleteObjectCmd : public EditCmdBase {
public:
    using EditCmdBase::EditCmdBase;

    const char* Label() const override { return "DeleteObjectCmd"; }

    bool Do(IDocument& p_doc) override;

    bool Undo(IDocument& p_doc) override;
};

class CloneObjectCmd : public EditCmdBase {
public:
    using EditCmdBase::EditCmdBase;

    const char* Label() const override { return "CloneObjectCmd"; }

    bool Do(IDocument& p_doc) override;

    bool Undo(IDocument& p_doc) override;
};

}  // namespace cave
