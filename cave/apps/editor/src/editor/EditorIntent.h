#pragma once
#include "cave/core/math/Vector.h"
#include "cave/runtime/intent/Intent.h"

#include "editor/edit/IEditCmd.h"

namespace cave {

class UndoIntent : public Intent {
public:
    CAVE_DECLARE_INTENT("editor.undo");

    UndoIntent(DocId p_doc_id)
        : doc_id(p_doc_id) {}

    DocId doc_id;
};

class RedoIntent : public Intent {
public:
    CAVE_DECLARE_INTENT("editor.redo");

    RedoIntent(DocId p_doc_id)
        : doc_id(p_doc_id) {}

    DocId doc_id;
};

class EditIntent : public Intent {
public:
    CAVE_DECLARE_INTENT("editor.edit");

    EditIntent(DocId p_doc_id, std::unique_ptr<IEditCmd>&& p_cmd)
        : doc_id(p_doc_id)
        , cmd(std::move(p_cmd)) {}

    const DocId doc_id;
    std::unique_ptr<IEditCmd> cmd;
};

class OpenDocIntent : public Intent {
public:
    CAVE_DECLARE_INTENT("editor.doc.open");

    OpenDocIntent(DocId p_doc_id)
        : doc_id(p_doc_id) {}

    DocId doc_id;
};

class CloseDocIntent : public Intent {
public:
    CAVE_DECLARE_INTENT("editor.doc.close");

    CloseDocIntent(DocId p_doc_id)
        : doc_id(p_doc_id) {}

    const DocId doc_id;
};

class SaveIntent : public Intent {
public:
    CAVE_DECLARE_INTENT("editor.doc.save");

    SaveIntent(bool p_save_as)
        : save_as(p_save_as) {}

    const bool save_as;
};

class PickIntent : public Intent {
public:
    CAVE_DECLARE_INTENT("editor.view.pick");

    PickIntent(math::Vector2f p_pointer)
        : pointer(p_pointer) {}

    const math::Vector2f pointer;
};

}  // namespace cave
