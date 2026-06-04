#pragma once
#include "cave/core/math/Vector.h"
#include "cave/runtime/intent/Intent.h"

#include "editor/edit/IEditCmd.h"

namespace cave {

class BaseDocIntent : public Intent {
public:
    BaseDocIntent(DocId p_doc_id)
        : doc_id(p_doc_id) {}

    DocId doc_id;

#if USING(DEBUG_BUILD)
    std::string DebugString() const override {
        return std::format("id=({},{})", doc_id.index, doc_id.gen);
    }
#endif
};

class OpenDocIntent : public BaseDocIntent {
public:
    CAVE_DECLARE_INTENT("editor.doc.open");

    using BaseDocIntent::BaseDocIntent;
};

class CloseDocIntent : public BaseDocIntent {
public:
    CAVE_DECLARE_INTENT("editor.doc.close");

    using BaseDocIntent::BaseDocIntent;
};

class UndoIntent : public BaseDocIntent {
public:
    CAVE_DECLARE_INTENT("editor.undo");

    using BaseDocIntent::BaseDocIntent;
};

class RedoIntent : public BaseDocIntent {
public:
    CAVE_DECLARE_INTENT("editor.redo");

    using BaseDocIntent::BaseDocIntent;
};

class SaveIntent : public BaseDocIntent {
public:
    CAVE_DECLARE_INTENT("editor.doc.save");

    SaveIntent(DocId p_doc_id, bool p_save_as)
        : BaseDocIntent(p_doc_id)
        , save_as(p_save_as) {}

    const bool save_as;
};

class EditIntent : public BaseDocIntent {
public:
    CAVE_DECLARE_INTENT("editor.edit");

    EditIntent(DocId p_doc_id, std::unique_ptr<IEditCmd>&& p_cmd)
        : BaseDocIntent(p_doc_id)
        , cmd(std::move(p_cmd)) {}

    std::unique_ptr<IEditCmd> cmd;
};

class PickIntent : public Intent {
public:
    CAVE_DECLARE_INTENT("editor.view.pick");

    PickIntent(math::Vector2f p_pointer)
        : pointer(p_pointer) {}

#if USING(DEBUG_BUILD)
    std::string DebugString() const override {
        return std::format("p=({},{})", pointer.x, pointer.y);
    }
#endif

    const math::Vector2f pointer;
};

}  // namespace cave
