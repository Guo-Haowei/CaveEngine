#pragma once
#include "cave/core/math/Vector.h"
#include "cave/runtime/intent/Intent.h"

#include "editor/edit/IEditCmd.h"

namespace cave {

class BaseDocIntent : public Intent {
public:
    BaseDocIntent(DocId doc_id)
        : doc_id_(doc_id) {}

    DocId doc_id() const { return doc_id_; }

#if USING(DEBUG_BUILD)
    std::string DebugString() const override {
        return std::format("id=({},{})", doc_id_.index, doc_id_.gen);
    }
#endif

protected:
    DocId doc_id_;
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

    SaveIntent(DocId doc_id, bool save_all)
        : BaseDocIntent(doc_id)
        , save_all_(save_all) {}

    bool save_all() const {
        return save_all_;
    }

private:
    bool save_all_;
};

class EditIntent : public BaseDocIntent {
public:
    CAVE_DECLARE_INTENT("editor.edit");

    EditIntent(DocId doc_id, std::unique_ptr<IEditCmd>&& cmd)
        : BaseDocIntent(doc_id)
        , cmd_(std::move(cmd)) {}

    std::unique_ptr<IEditCmd> cmd_;
};

class PickIntent : public Intent {
public:
    CAVE_DECLARE_INTENT("editor.view.pick");

    PickIntent(math::Vec2f pointer)
        : pointer_(pointer) {}

    math::Vec2f pointer() const { return pointer_; }

#if USING(DEBUG_BUILD)
    std::string DebugString() const override {
        return std::format("p=({},{})", pointer_.x, pointer_.y);
    }
#endif
private:
    math::Vec2f pointer_;
};

}  // namespace cave
