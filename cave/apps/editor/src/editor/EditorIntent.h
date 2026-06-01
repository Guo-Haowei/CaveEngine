#pragma once
#include "cave/runtime/intent/Intent.h"

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

}  // namespace cave
