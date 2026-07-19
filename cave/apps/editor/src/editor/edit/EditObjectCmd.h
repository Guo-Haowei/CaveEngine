#pragma once
#include "EditCmdBase.h"

#include "editor/document/SceneDocument.h"

namespace cave {

class DeleteObjectCmd : public EditCmdBase {
public:
    DeleteObjectCmd(SceneRegistry& scene_reg, ecs::Entity ent)
        : EditCmdBase(scene_reg)
        , m_ent(ent) {}

    const char* label() const override { return "DeleteObjectCmd"; }

    bool apply(IDocument& doc) override;

    bool undo(IDocument& doc) override;

private:
    ecs::Entity m_ent;
};

class CloneObjectCmd : public EditCmdBase {
public:
    CloneObjectCmd(SceneRegistry& scene_reg, ecs::Entity ent)
        : EditCmdBase(scene_reg)
        , m_ent(ent) {}

    const char* label() const override { return "CloneObjectCmd"; }

    bool apply(IDocument& doc) override;

    bool undo(IDocument& doc) override;

private:
    ecs::Entity m_ent;
};

}  // namespace cave
