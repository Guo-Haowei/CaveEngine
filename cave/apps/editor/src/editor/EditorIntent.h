#pragma once
#include "cave/core/math/Vec.h"
#include "cave/runtime/intent/Intent.h"

#include "editor/edit/IEditCmd.h"

namespace cave {

class BaseDocIntent : public Intent {
public:
    BaseDocIntent(DocId doc_id)
        : m_doc_id(doc_id) {}

    DocId doc_id() const { return m_doc_id; }

#if USING(DEBUG_BUILD)
    std::string debugString() const override {
        return m_doc_id.toString();
    }
#endif

protected:
    DocId m_doc_id;
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

    SaveIntent(DocId doc_id)
        : BaseDocIntent(doc_id) {}
};

class EditIntent : public BaseDocIntent {
public:
    CAVE_DECLARE_INTENT("editor.edit");

    EditIntent(DocId doc_id, Owner<IEditCmd>&& cmd)
        : BaseDocIntent(doc_id)
        , m_cmd(std::move(cmd)) {}

    std::unique_ptr<IEditCmd> m_cmd;
};

class PickIntent : public Intent {
public:
    CAVE_DECLARE_INTENT("editor.view.pick");

    PickIntent(math::Vec2f pointer)
        : m_pointer(pointer) {}

    math::Vec2f pointer() const { return m_pointer; }

#if USING(DEBUG_BUILD)
    std::string debugString() const override {
        return std::format("p=({},{})", m_pointer.x, m_pointer.y);
    }
#endif
private:
    math::Vec2f m_pointer;
};

}  // namespace cave
