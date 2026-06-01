#pragma once
#include "cave/runtime/intent/Intent.h"

#include "editor/edit/IEditCmd.h"

namespace cave {

class SaveIntent : public Intent {
public:
    CAVE_DECLARE_INTENT("cave.editor.save");

    SaveIntent(bool p_save_as)
        : m_save_as(p_save_as) {}

    bool SaveAs() const { return m_save_as; }

private:
    const bool m_save_as{ false };
};

class UndoIntent : public Intent {
public:
    CAVE_DECLARE_INTENT("cave.editor.undo");
};

class RedoIntent : public Intent {
public:
    CAVE_DECLARE_INTENT("cave.editor.redo");
};

class EditIntent : public Intent {
public:
    CAVE_DECLARE_INTENT("cave.editor.edit");

    EditIntent(DocId p_doc_id, std::unique_ptr<IEditCmd>&& p_cmd)
        : doc_id(p_doc_id)
        , cmd(std::move(p_cmd)) {}

    DocId doc_id;
    std::unique_ptr<IEditCmd> cmd;
};

}  // namespace cave
