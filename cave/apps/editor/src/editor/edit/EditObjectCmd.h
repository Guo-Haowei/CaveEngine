#pragma once
#include "EditCmdBase.h"

#include "editor/document/SceneDocument.h"

namespace cave {

class DeleteObjectCmd : public EditCmdBase {
public:
    using EditCmdBase::EditCmdBase;

    const char* label() const override { return "DeleteObjectCmd"; }

    bool apply(IDocument& doc) override;

    bool undo(IDocument& doc) override;
};

class CloneObjectCmd : public EditCmdBase {
public:
    using EditCmdBase::EditCmdBase;

    const char* label() const override { return "CloneObjectCmd"; }

    bool apply(IDocument& doc) override;

    bool undo(IDocument& doc) override;
};

}  // namespace cave
