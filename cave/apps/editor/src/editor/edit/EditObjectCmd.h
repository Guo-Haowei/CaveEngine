#pragma once
#include "EditCmdBase.h"

#include "editor/document/SceneDocument.h"

namespace cave {

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
