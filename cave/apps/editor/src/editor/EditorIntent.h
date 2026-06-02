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

    UndoIntent(DocId p_doc_id)
        : doc_id(p_doc_id) {}

    DocId doc_id;
};

class RedoIntent : public Intent {
public:
    CAVE_DECLARE_INTENT("cave.editor.redo");

    RedoIntent(DocId p_doc_id)
        : doc_id(p_doc_id) {}

    DocId doc_id;
};

class EditIntent : public Intent {
public:
    CAVE_DECLARE_INTENT("cave.editor.edit");

    EditIntent(DocId p_doc_id, std::unique_ptr<IEditCmd>&& p_cmd)
        : doc_id(p_doc_id)
        , cmd(std::move(p_cmd)) {}

    const DocId doc_id;
    std::unique_ptr<IEditCmd> cmd;
};

class OpenDocIntent : public Intent {
public:
    CAVE_DECLARE_INTENT("cave.doc.open");

    OpenDocIntent(DocId p_doc_id)
        : doc_id(p_doc_id) {}

    DocId doc_id;
};

class CloseDocIntent : public Intent {
public:
    CAVE_DECLARE_INTENT("cave.doc.close");

    CloseDocIntent(DocId p_doc_id)
        : doc_id(p_doc_id) {}

    const DocId doc_id;
};

}  // namespace cave
